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

kernel void sort( global float8 * vectors, global float8 * bboxes, int obj_n, global int * sortbuf )
{
	int i = get_global_id(1)*DIM_X + get_global_id(0);
	
	/*Power too weak? Don't bother.*/
	if ( vectors[i].s3 < 0.05 )
	{
		sortbuf[i*4+0] =-1;
		return;
	}
	
	float3 pos = vectors[i].s012;
	float3 dir = vectors[i].s456;
	float3 invdir = 1.0f/dir.xyz;
	int4 sbuf = (int4)(-1, -1, -1, -1 );
	float4 fbuf = (float4)( 32768.0f, 32768.0f, 32768.0f, 32768.0f );
	
	for( int o=0; o<obj_n; o++ )
	{
		float t;
		if ( bb_test( bboxes[o].s012, bboxes[o].s456, invdir, pos, &t ) == 1 )
		{
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
