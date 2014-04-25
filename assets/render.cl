constant sampler_t sampler = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_REPEAT | CLK_FILTER_NEAREST;

typedef struct objec_struct
{
	float4 bba;
	float4 bbb;
	int nTri;
	int offset;
	int2 padding;
} Object;

#define EPSILON 0.00001f
int intersect_tri( float3 v1, float3 v2, float3 v3, float3 ori, float3 dir, float4 * out )
{
	/*Möller-Trumbore's algorithm*/
	/*http://www.graphics.cornell.edu/pubs/1997/MT97.html*/
	
	float3 e1, e2;
	float3 P, Q, T;
	float det, inv_det, u, v;
	float t;
	
	e1 = v2 - v1;
	e2 = v3 - v1;
	
	P = cross( dir, e2 );
	det = dot( e1, P );
	
	if ( det >-EPSILON && det < EPSILON )
		return -1.0f;
	
	inv_det = 1.0f/det;
	
	T = ori - v1;
	u = dot( T, P ) * inv_det;
	
	if( u < 0.f || u > 1.0f )
		return 0;
	
	Q = cross( T, e1 );
	v = dot( dir, Q ) * inv_det;
	if( v < 0.0f || u + v  > 1.0f ) return -1.0f;
	
	t = dot( e2, Q ) * inv_det;
	if( t > EPSILON )
	{
		(*out).s0123 = (float4)( 1.0f-u-v, u, v, t );
		return 1;
	}
	return 0;
}

float3 normal_interpolate( global float8 * tri, float3 bary )
{
	return tri[0].s345*bary.x + tri[1].s345*bary.y + tri[2].s345*bary.z;
}

float2 uv_interpolate( global float8 * tri, float3 bary )
{
	return tri[0].s67*bary.x + tri[1].s67*bary.y + tri[2].s67*bary.z;
}

int ray_trace_triangles( global float8 * tri, int nTri, float3 * ori, float3 * dir, float3 * col, float * str, read_only image2d_t dTex )
{
	int mi = -1;
	float4 mout = (float4)( 0.0f, 0.0f, 0.0f, 32.0f*1024.0f );
	
	for( int i=0; i<nTri; i++ )
	{
		float4 result;
		if ( intersect_tri( tri[i*3+0].xyz, tri[i*3+1].xyz, tri[i*3+2].xyz, *ori, *dir, &result ) == 1 )
		{
			if( result.s3 < mout.s3 )
			{
				mi = i;
				mout = result;
			}
		}
	}
	
	if ( mi != -1 )
	{
		/*Calculate a new vector*/
		float3 npos = *ori + *dir * mout.s3*0.9999f;
		float3 normal = normal_interpolate( tri+mi*3, mout.xyz );
		float2 uv = uv_interpolate( tri+mi*3, mout.xyz );
		float3 ref = *dir - 2.0f*(dot(*dir, normal))*normal;
		//float3 ref = normalize(dir + (0.0f)*normal);
		
		/*Diffuse+reflect texture*/
		float4 difref = read_imagef( dTex, sampler, uv );
		
		*col += (float3)( difref.xyz )*(*str)*(difref.s3);
		*str = (*str)*(1.0-difref.s3);
		*ori = npos;
		*dir = ref;
		return 1;
	}
	return 0;
}

float sky_color( float3 vec )
{
	return vec.z*0.5f+0.5f;
}

int bb_test( float3 lb, float3 rt, float3 invdir, float3 ori, float * t )
{
	float t1 = (lb.x - ori.x)*invdir.x;
	float t2 = (rt.x - ori.x)*invdir.x;
	float t3 = (lb.y - ori.y)*invdir.y;
	float t4 = (rt.y - ori.y)*invdir.y;
	float t5 = (lb.z - ori.z)*invdir.z;
	float t6 = (rt.z - ori.z)*invdir.z;
	float tmin = max(max(min(t1, t2), min(t3, t4)), min(t5, t6));
	float tmax = min(min(max(t1, t2), max(t3, t4)), max(t5, t6));
	if ( tmax < 0.0f )
	{
		*t = tmax;
		return 0;
	}
	if (tmin > tmax)
	{
		*t = tmax;
		return 0;
	}
	*t = tmin;
	return 1;
}

void obj_sort_partial( global Object * objects, int nObjects, float3 invdir, float3 ray_ori, int * dsort )
{
	/*Only sort 4 objects, for performance reasons - if objects are filled with holes, there might be graphics bugs.*/
	float lengths[4] = { 32.0f*1024.0f, 32.0f*1024.0f, 32.0f*1024.0f, 32.0f*1024.0f };
	
	float tlen;
	for( int obj=0; obj<nObjects; obj++ )
	{
		if ( bb_test( objects[obj].bba.xyz, objects[obj].bbb.xyz, invdir, ray_ori, &tlen ) == 1 )
		{
			if ( tlen < lengths[0] )
			{
				lengths[3] = lengths[2];
				lengths[2] = lengths[1];
				lengths[1] = lengths[0];
				lengths[0] = tlen;
				dsort[3] = dsort[2];
				dsort[2] = dsort[1];
				dsort[1] = dsort[0];
				dsort[0] = obj;
			}
			else if ( tlen < lengths[1] )
			{
				lengths[3] = lengths[2];
				lengths[2] = lengths[1];
				lengths[1] = tlen;
				dsort[3] = dsort[2];
				dsort[2] = dsort[1];
				dsort[1] = obj;
			}
			else if ( tlen < lengths[2] )
			{
				lengths[3] = lengths[2];
				lengths[2] = tlen;
				dsort[3] = dsort[2];
				dsort[2] = obj;
			}
			else if ( tlen < lengths[3] )
			{
				lengths[3] = tlen;
				dsort[3] = obj;
			}
		}
	}
}

float3 trace( float3 ray_ori, float3 ray_dir, int nObjects, global Object * objects, global float8 * tri, read_only image2d_t diffuse_t )
{
	float3 ray_col = (float3)( 0.0f );
	float str = 1.0f;
	int hit = 0;
	
	for( int i=0; i<3; i++ )
	{
		float3 invdir = 1.0f/ray_dir;
		int dsort[4] = { -1, -1, -1, -1 };
		hit = 0;
		
		obj_sort_partial( objects, nObjects, invdir, ray_ori, dsort );
	
		for( int ot=0; ot<3; ot++ )
		{
			if ( dsort[ot] == -1 )
			{
				return ray_col + sky_color( ray_dir )*str;
			}
			
			if( ray_trace_triangles( tri+objects[dsort[ot]].offset, objects[dsort[ot]].nTri, &ray_ori, &ray_dir, &ray_col, &str, diffuse_t ) == 1 )
			{
				if ( str< 0.02 )
				{
					return ray_col;
				}
				hit = 1;
				break;
			}
		}

	}
	if ( hit == 0 )
		return ray_col + sky_color( ray_dir )*str;
	
	return ray_col;
}

kernel void core( write_only image2d_t image, constant float4 * camera, int nObjects, global Object * objects, global float8 * tri, read_only image2d_t diffuse_t )
{
	int2 cordi = (int2)( get_global_id(0), get_global_id(1) );
	float2 cordf = (float2)( cordi.x/(DIM_X), cordi.y/(DIM_Y) );
	
	/*Setup the ray vector & colour*/
	float3 ray_ori = ( camera[1] + camera[2]*cordf.x+camera[3]*cordf.y ).xyz;
	float3 ray_dir = normalize( ray_ori - camera[0].xyz ); 
	
	/*Trace!*/
	float3 ray_col = trace( ray_ori, ray_dir, nObjects, objects, tri, diffuse_t );
	
	/*Write final colour to texture*/
	write_imagef( image, cordi, (float4)(ray_col, 1.0f) );
}
