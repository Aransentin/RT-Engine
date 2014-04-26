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
		return -1.0f;
	
	inv_det = 1.0f/det;
	
	T = ori - v1;
	u = dot( T, P ) * inv_det;
	
	if( u < 0.f || u > 1.0f )
		return 0;
	
	Q = cross( T, e1 );
	v = dot( dir, Q ) * inv_det;
	if( v < 0.0f || u + v  > 1.0f ) return -1.0f;
	
	t = dot( e2, Q ) * inv_det;
	if( t > EPSILON )
	{
		(*out).s0123 = (float4)( 1.0f-u-v, u, v, t );
		return 1;
	}
	return 0;
}

float3 normal_interpolate( global float8 * tri, float3 bary )
{
	return tri[0].s345*bary.x + tri[1].s345*bary.y + tri[2].s345*bary.z;
}

float2 uv_interpolate( global float8 * tri, float3 bary )
{
	return tri[0].s67*bary.x + tri[1].s67*bary.y + tri[2].s67*bary.z;
}

int ray_trace_triangles( global float8 * tri, int nTri, float3 ori, float3 dir, global float16 * out )
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
	{
		float3 npos = ori + mout.s3*dir*0.9999f;
		float3 nnor = normal_interpolate( tri+mi*3, mout.xyz );
		float3 ndir = dir - 2.0f*(dot( dir, nnor ) )*nnor;
		float2 nuv = uv_interpolate(  tri+mi*3, mout.xyz );
		
		out[0].s012 = npos;
		out[0].s456 = ndir;
		out[0].s89A = nnor;
		out[0].sCD = nuv;
		
		return 1;
	}
	return 0;
}

void nope( global float16 * out )
{
	out[0] = (float16)(0.0f);
}

kernel void trace( global float8 * vec, global int * sortbuf, global int2 * objects, global float8 * vert, global float16 * out )
{
	int i = get_global_id(1)*DIM_X + get_global_id(0);
	
	for( int o=0; o<4; o++ )
	{
		int oid = sortbuf[i*4+o];
		if ( oid == -1 )
		{
			nope( out + i );
			return;
		}
		else
		{
			if( ray_trace_triangles( vert + objects[oid].s0, objects[oid].s1, vec[i].s012, vec[i].s456, out + i ) == 1 )
			{
				out[i].sE = vec[i].s3;
				return;
			}
		}
	}
	nope( out + i );
	
	
	return;
}