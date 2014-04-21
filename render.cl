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

int ray_trace( global float8 * tri, float3 ori, float3 dir, float3 * ori_out, float3 * dir_out, float3 * col_out )
{
	int mi = -1;
	float4 mout = (float4)( 0.0, 0.0, 0.0, 8.0*2048.0 );
	
	/*Test every triangle in the world - must be improved*/
	for( int i=0; i<3270/3; i++ )
	{
		float4 result;
		if ( intersect_tri( tri[i*3+0].xyz, tri[i*3+1].xyz, tri[i*3+2].xyz, ori, dir, &result ) == 1 )
		{
			if( result.s3 < mout.s3 )
			{
				mi = i;
				mout = result;
			}
		}
	}
	
	if ( mi == -1 )
	{
		*col_out = (float3)( 1.0, 1.0, 1.0 )*(0.2+dir.z);
		return 0;
	}
	else
	{
		/*Calculate a new vector*/
		float3 npos = ori + dir * mout.s3;
		float3 normal = normal_interpolate( tri, mi, mout.xyz );
		float2 uv = uv_interpolate( tri, mi, mout.xyz );
		float3 ref = dir - 2.0*(dot(dir, normal))*normal;
		
		
		*ori_out = npos;
		*dir_out = ref;
		
		*col_out = (float3)( uv, 0.3 );
		return 1;
	}
}

kernel void core( write_only image2d_t image, constant float4 * camera, global float8 * tri )
{
	int2 cordi = (int2)( get_global_id(0), get_global_id(1) );
	float2 cordf = (float2)( cordi.x/DIM_X, cordi.y/DIM_Y );
	
	/*Setup the ray vector & colour*/
	float3 ray_ori = ( camera[1] + camera[2]*cordf.x+camera[3]*cordf.y ).xyz;
	float3 ray_dir = normalize( ray_ori - camera[0].xyz ); 
	float3 ray_col = (float3)( 0.0, 0.0, 0.0 );
	
	/*Trace the ray, bounce three times at maximum*/
	for( int i=0; i<4; i++ )
	{
		float3 ray_ori_new, ray_dir_new, col_new;
		
		if ( ray_trace( tri, ray_ori, ray_dir, &ray_ori_new, &ray_dir_new, &col_new ) )
		{
			ray_col += col_new*( 1.0/(i+1) );
			ray_dir = ray_dir_new;
			ray_ori = ray_ori_new;
		}
		else
		{
			ray_col += col_new*( 1.0/(i+1) );
			break;
		}
	}
	
	/*Write final colour to texture*/
	write_imagef( image, cordi, (float4)(ray_col, 1.0) );
}
