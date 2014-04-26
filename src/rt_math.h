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

static inline void mat4_identity( float m[16] )
{
	m[0]  = 1.0f; m[1]  = 0.0f; m[2]  = 0.0f; m[3]  = 0.0f;
	m[4]  = 0.0f; m[5]  = 1.0f; m[6]  = 0.0f; m[7]  = 0.0f;
	m[8]  = 0.0f; m[9]  = 0.0f; m[10] = 1.0f; m[11] = 0.0f;
	m[12] = 0.0f; m[13] = 0.0f; m[14] = 0.0f; m[15] = 1.0f;
}

static inline void mat4_translate( float m[16], float x, float y, float z )
{
	m[12] += x;
	m[13] += y;
	m[14] += z;
}

static inline void mat4_mul( float m[16], const float mat_a[16], const float mat_b[16] )
{
	float tmp_m[16];
	int i;
	for( i=0; i<4; i++ )
	{
		tmp_m[ 0+i] = mat_a[0+i]*mat_b[0]  + mat_a[4+i]*mat_b[1]  + mat_a[8+i]*mat_b[2]  + mat_a[12+i]*mat_b[3];
		tmp_m[ 4+i] = mat_a[0+i]*mat_b[4]  + mat_a[4+i]*mat_b[5]  + mat_a[8+i]*mat_b[6]  + mat_a[12+i]*mat_b[7];
		tmp_m[ 8+i] = mat_a[0+i]*mat_b[8]  + mat_a[4+i]*mat_b[9]  + mat_a[8+i]*mat_b[10] + mat_a[12+i]*mat_b[11];
		tmp_m[12+i] = mat_a[0+i]*mat_b[12] + mat_a[4+i]*mat_b[13] + mat_a[8+i]*mat_b[14] + mat_a[12+i]*mat_b[15];
	}
	for( i=0; i<16; i++ )
		m[i] = tmp_m[i];
}

static inline void mat4_vec3_mul( float r[3], float mat[16], float vec[3] )
{
	float tmp_v[3];
	
	tmp_v[0] = mat[0]*vec[0] + mat[4]*vec[1] + mat[8]*vec[2] + mat[12];
	tmp_v[1] = mat[1]*vec[0] + mat[5]*vec[1] + mat[9]*vec[2] + mat[13];
	tmp_v[2] = mat[2]*vec[0] + mat[6]*vec[1] + mat[10]*vec[2] + mat[14];
	
	for( int i=0; i<3; i++ )
		r[i] = tmp_v[i];
}

static inline void mat4_nvec3_mul( float r[3], float mat[16], float vec[3] )
{
	float tmp_v[3];
	
	tmp_v[0] = mat[0]*vec[0] + mat[4]*vec[1] + mat[8]*vec[2];
	tmp_v[1] = mat[1]*vec[0] + mat[5]*vec[1] + mat[9]*vec[2];
	tmp_v[2] = mat[2]*vec[0] + mat[6]*vec[1] + mat[10]*vec[2];
	
	for( int i=0; i<3; i++ )
		r[i] = tmp_v[i];
}

static inline void mat4_rotate_X( float out[16], float mat[16], float angle )
{
	float s = sinf( angle );
	float c = cosf( angle );
	float r[16] = 
	{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f,    c,    s, 0.0f,
		0.0f,   -s,    c, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	mat4_mul( out, mat, r );
}

static inline void mat4_rotate_Y( float out[16], float mat[16], float angle )
{
	float s = sinf( angle );
	float c = cosf( angle );
	float r[16] = 
	{
		   c, 0.0f,    s, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		  -s, 0.0f,    c, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	mat4_mul( out, mat, r );
}

static inline void mat4_rotate_Z( float out[16], float mat[16], float angle )
{
	float s = sinf( angle );
	float c = cosf( angle );
	float r[16] = 
	{
		   c,    s, 0.0f, 0.0f,
		  -s,    c, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	mat4_mul( out, mat, r );
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
