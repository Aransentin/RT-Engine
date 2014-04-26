constant sampler_t sampler = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_REPEAT | CLK_FILTER_NEAREST;

kernel void compose( global float16 * buf, read_only image2d_t texture, write_only global float4 * out, global float8 * nrays, global float4 * light )
{
	int i = get_global_id(1)*DIM_X + get_global_id(0);
	
	/*Diffuse+reflectivity*/
	float4 difref = read_imagef( texture, sampler, buf[i].sCD );
	float str = buf[i].E;
	float lightval = light[i].s3;
	float ldot = max( dot( light[i].s012, buf[i].s89a ), 0.0f );
	
	out[i] += (float4)( difref.xyz, 0.0 )*(difref.s3 )*str*lightval*ldot;
	
	/*Write to new ray buffer*/
	nrays[i].s0123 = (float4)( buf[i].s012, (1.0f-difref.s3)*str );
	nrays[i].s456 = (float3)( buf[i].s456 );
}