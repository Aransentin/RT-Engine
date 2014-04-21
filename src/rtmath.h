#pragma once

#include <math.h>
#include <string.h>

static inline void vec3_normalize( float res[3], const float vec[3] )
{
	float nval = sqrtf( vec[0]*vec[0]+vec[1]*vec[1]+vec[2]*vec[2] );

	if ( nval > 0.00001 || nval < -0.00001 )
	{
		res[0] = vec[0]/nval;
		res[1] = vec[1]/nval;
		res[2] = vec[2]/nval;
	}
}

static inline void vec3_cross( float res[3], const float vec_a[3], const float vec_b[3] )
{
	res[0] = vec_a[1] * vec_b[2] - vec_a[2] * vec_b[1];
	res[1] = vec_a[2] * vec_b[0] - vec_a[0] * vec_b[2];
	res[2] = vec_a[0] * vec_b[1] - vec_a[1] * vec_b[0];
}

static inline void rtmath_lookTo( float cbuf[16], float pos[3], float dir[3], float nor[3], float z, float aspect )
{
	memcpy( cbuf, pos, sizeof(float)*3 );

	float mvec[3];
	vec3_normalize( dir, dir );
	vec3_cross( mvec, dir, nor );
	
	cbuf[3] = 0.0f;
	cbuf[4] = pos[0]+dir[0]*z - 0.5f*mvec[0]*aspect - 0.5f*nor[0];
	cbuf[5] = pos[1]+dir[1]*z - 0.5f*mvec[1]*aspect - 0.5f*nor[1];
	cbuf[6] = pos[2]+dir[2]*z - 0.5f*mvec[2]*aspect - 0.5f*nor[2];
	cbuf[7] = 0.0f;
	cbuf[8] = mvec[0]*aspect;
	cbuf[9] = mvec[1]*aspect;
	cbuf[10] = mvec[2]*aspect;
	cbuf[11] = 0.0f;
	cbuf[12] = nor[0];
	cbuf[13] = nor[1];
	cbuf[14] = nor[2];
	cbuf[15] = 0.0f;
}
