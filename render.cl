#define EPSILON 0.00001f
float MT_intersect_tri( float3 v1, float3 v2, float3 v3, float3 ori, float3 dir )
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
		return -1.0f;
	
	Q = cross( T, e1 );
	v = dot( dir, Q ) * inv_det;
	if( v < 0.0f || u + v  > 1.0f ) return -1.0;
	
	t = dot( e2, Q ) * inv_det;
	if( t > EPSILON )
	{
		return t;
	}
	return -1.0f;
}

kernel void core( write_only image2d_t image, constant float4 * camera, global float8 * tri )
{
	int2 cordi = (int2)( get_global_id(0), get_global_id(1) );
	float2 cordf = (float2)( cordi.x/DIM_X, cordi.y/DIM_Y );
	
	float3 ray_ori = ( camera[1] + camera[2]*cordf.x+camera[3]*cordf.y ).xyz;
	float3 ray_dir = normalize( ray_ori - camera[0].xyz ); 
	
	float max_d = -1.0f;
	int max_i = -1;
	for( int i=0; i<968; i++ )
	{
		float test = MT_intersect_tri( tri[i*3+0].xyz, tri[i*3+1].xyz, tri[i*3+2].xyz, ray_ori, ray_dir );
		
		if ( test > max_d )
		{
			max_i = i;
			max_d = test;
		}
	}
	if ( max_i != -1 )
	{
		float4 colour = (float4)( tri[max_i*3+0].s3, tri[max_i*3+0].s4, tri[max_i*3+0].s5, 255 );
		write_imagef( image, cordi, colour );
		return;
	}
	float4 colour = (float4)( 0.0, 0.0, 0.0, 255 );
	write_imagef( image, cordi, colour );
}