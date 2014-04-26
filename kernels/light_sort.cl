kernel void lightsort( global float16 * in, global float8 * bboxes, int obj_n, float4 lightpos, global int * sortbuf )
{
	int i = get_global_id(1)*DIM_X + get_global_id(0);
	
	float3 pos = in[i].xyz;
	float3 dir = normalize( lightpos.xyz - pos );
	float len = length( lightpos.xyz - pos );
	float3 invdir = 1.0f/dir;
	int4 sbuf = (int4)(-1, -1, -1, -1 );
	float4 fbuf = (float4)( 32768.0f, 32768.0f, 32768.0f, 32768.0f );
	
	for( int o=0; o<obj_n; o++ )
	{
		float t;
		if ( bb_test( bboxes[o].s012, bboxes[o].s456, invdir, pos, &t ) == 1 )
		{
			//if ( t > len )
			//	continue;
			
			if ( t < fbuf.s0 )
			{
				sbuf.s123 = sbuf.s012;
				fbuf.s123 = fbuf.s012;
				fbuf.s0 = t;
				sbuf.s0 = o;
			}
			else if ( t < fbuf.s1 )
			{
				sbuf.s23 = sbuf.s12;
				fbuf.s23 = fbuf.s12;
				fbuf.s1 = t;
				sbuf.s1 = o;
			}
			else if ( t < fbuf.s2 )
			{
				sbuf.s3 = sbuf.s2;
				fbuf.s3 = fbuf.s2;
				fbuf.s2 = t;
				sbuf.s2 = o;
			}
			else if ( t< fbuf.s3 )
			{
				fbuf.s3 = t;
				sbuf.s3 = o;
			}
		}
	}
	sortbuf[i*4+0] = sbuf.s0;
	sortbuf[i*4+1] = sbuf.s1;
	sortbuf[i*4+2] = sbuf.s2;
	sortbuf[i*4+3] = sbuf.s3;
}