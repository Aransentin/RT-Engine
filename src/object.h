#pragma once

typedef struct object_struct Object;
typedef struct object_struct_gpu ObjectGPU;

#include "mesh.h"
#include <CL/cl.h>
#include <CL/cl_gl.h>

struct object_struct
{
	Mesh * mesh;
	unsigned int buffer_offset;
	char dynamic;
	char padding[3];
};

struct object_struct_gpu
{
	cl_float4 bba;
	cl_float4 bbb;
	cl_int nTri;
	cl_int offset;
	cl_int2 padding;
};

Object * object_new( void );
