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
		return -1.0;
	
	inv_det = 1.0f/det;
	
	T = ori - v1;
	u = dot( T, P ) * inv_det;
	
	if( u < 0.f || u > 1.0f )
		return 0;
	
	Q = cross( T, e1 );
	v = dot( dir, Q ) * inv_det;
	if( v < 0.0f || u + v  > 1.0f ) return -1.0;
	
	t = dot( e2, Q ) * inv_det;
	if( t > EPSILON )
	{
		(*out).s0123 = (float4)( 1.0-u-v, u, v, t );
		return 1;
	}
	return 0;
}

float3 normal_interpolate( global float8 * tri, int t, float3 bary )
{
	return tri[t*3+0].s345*bary.x + tri[t*3+1].s345*bary.y + tri[t*3+2].s345*bary.z;
}
float2 uv_interpolate( global float8 * tri, int t, float3 bary )
{
	return tri[t*3+0].s67*bary.x + tri[t*3+1].s67*bary.y + tri[t*3+2].s67*bary.z;
}

int ray_trace( global float8 * tri, float3 * ori, float3 * dir, float3 * col, float str )
{
	int mi = -1;
	float4 mout = (float4)( 0.0, 0.0, 0.0, 8.0*2048.0 );
	
	for( int i=0; i<1692/3; i++ )
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
		float3 npos = *ori + *dir * mout.s3*0.9999;
		float3 normal = normal_interpolate( tri, mi, mout.xyz );
		float2 uv = uv_interpolate( tri, mi, mout.xyz );
		float3 ref = *dir - 2.0*(dot(*dir, normal))*normal;
		//float3 ref = normalize(dir + (0.0f)*normal);
		
		*col += (float3)( uv*0.5, 0.15 )*str;
		*ori = npos;
		*dir = ref;
		return 1;
	}
	return 0;
}

float sky_color( float3 vec )
{
	return vec.z*0.2+0.3;
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

kernel void core( write_only image2d_t image, constant float4 camera[4], global float8 * tri )
{
	int2 cordi = (int2)( get_global_id(0), get_global_id(1) );
	float2 cordf = (float2)( cordi.x/(DIM_X), cordi.y/(DIM_Y) );
	
	/*Setup the ray vector & colour*/
	float3 ray_ori = ( camera[1] + camera[2]*cordf.x+camera[3]*cordf.y ).xyz;
	float3 ray_dir = normalize( ray_ori - camera[0].xyz ); 
	float3 ray_col = (float3)( 0.0, 0.0, 0.0 );
	
	/*Trace the ray, bounce three times at maximum*/
	for( int i=0; i<3; i++ )
	{
		float tlen;
		float3 invdir = 1.0f/ray_dir;
		if ( bb_test( (float3)( -2.4f, -2.4f, -3.7f ), (float3)( 2.3f, 2.3f, 2.2f ), invdir, ray_ori, &tlen ) == 1 )
		{
			if( ray_trace( tri, &ray_ori, &ray_dir, &ray_col, 1.0/(i+1) ) != 1 )
			{
				ray_col += (float3)(sky_color( ray_dir ));
				break;
			}
		}
		else
		{
			ray_col += (float3)(sky_color( ray_dir ));
			break;
		}
	}
	
	/*Write final colour to texture*/
	write_imagef( image, cordi, (float4)(ray_col, 1.0) );
}
