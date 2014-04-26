kernel void generate_rays( float4 ori, float4 corner, float4 dir1, float4 dir2, global float8 * out )
{
	float2 cordf = (float2)( get_global_id(0)/(float)DIM_X, get_global_id(1)/(float)DIM_Y );
	
	float4 ray_ori = corner + dir1*cordf.x + dir2*cordf.y;
	float4 ray_dir = normalize( ray_ori - ori ); 
	ray_ori.s3 = 1.0f;
	
	int i = get_global_id(1)*DIM_X + get_global_id(0);
	out[i] = (float8)( ray_ori, ray_dir );
}
