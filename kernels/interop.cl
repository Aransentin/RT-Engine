kernel void interop( global float4 * in, write_only image2d_t out )
{
	int i = get_global_id(1)*DIM_X + get_global_id(0);
	
	write_imagef( out, (int2)( get_global_id(0), get_global_id(1) ), in[i] );
}