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

kernel void core( write_only image2d_t image, constant float4 * camera, global float8 * tri )
{
	int2 cordi = (int2)( get_global_id(0), get_global_id(1) );
	float2 cordf = (float2)( cordi.x/DIM_X, cordi.y/DIM_Y );
	
	float3 ray_ori = ( camera[1] + camera[2]*cordf.x+camera[3]*cordf.y ).xyz;
	float3 ray_dir = normalize( ray_ori - camera[0].xyz ); 
	
	int max_i = -1;
	float4 max_out;
	
	for( int i=0; i<2512; i++ )
	{
		float4 test;
		int status = intersect_tri( tri[i*3+0].xyz, tri[i*3+1].xyz, tri[i*3+2].xyz, ray_ori, ray_dir, &test );
		
		if ( status == 1 && test.s3 > max_out.s3 )
		{
			max_i = i;
			max_out = test;
		}
	}
	if ( max_i != -1 )
	{
		float3 npos = ray_ori + ray_dir*max_out.s3;
		
		//float4 colour = (float4)( tri[max_i*3+0].s3, tri[max_i*3+0].s4, tri[max_i*3+0].s5, 255 );
		
		float3 normals = tri[max_i*3+0].s345*max_out.x + tri[max_i*3+1].s345*max_out.y + tri[max_i*3+2].s345*max_out.z;
		
		normals = normals*0.5 + 0.5;
		
		float4 colour = (float4)( normals, 255 );
		write_imagef( image, cordi, colour );
		return;
	}
	else
	{
		//float4 colour = (float4)( 0.5, 0.5, 0.7+max(ray_dir.z, 0.0), 255 );
		float4 colour = (float4)( 0.0, 0.0, 0.0, 255 );
		
		write_imagef( image, cordi, colour );
	}
}