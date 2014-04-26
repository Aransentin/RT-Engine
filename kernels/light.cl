
int ray_trace_light( global float8 * tri, int nTri, float3 ori, float3 dir )
{
	int mi = -1;
	float4 mout = (float4)( 0.0f, 0.0f, 0.0f, 32.0f*1024.0f );
	
	for( int i=0; i<nTri; i++ )
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
	
	if ( mi != -1 )
		return 1;
	else
		return 0;
}

kernel void light( global float16 * in, float4 light_pos, global float4 * out, global int * sortbuf, global int2 * objects, global float8 * vert )
{
	int i = get_global_id(1)*DIM_X + get_global_id(0);
	
	float shadowVal = 1.0f;
	float3 dir = normalize( light_pos.xyz - in[i].xyz );

	for( int o=0; o<4; o++ )
	{
		int oid = sortbuf[i*4+o];
		if ( oid == -1 )
		{
			break;
		}
		else
		{
			if( ray_trace_light( vert + objects[oid].s0, objects[oid].s1, in[i].s012, dir ) == 1 )
			{
				shadowVal = 0.2f;
				break;
			}
		}
	}
	
	out[i].s012 = dir;
	out[i].s3 = shadowVal*max( 5.0f - 0.3f*length( light_pos.xyz - in[i].xyz ), 0.0f );
}