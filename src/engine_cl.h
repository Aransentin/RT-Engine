#pragma once

typedef struct engine_cl_struct Engine_cl;

#include <CL/cl.h>
#include <CL/cl_gl.h>

#include "world.h"

struct engine_cl_struct
{
	cl_platform_id platform;
	cl_device_id device;
	cl_context context;
	cl_command_queue queue;
	cl_program program;
	cl_kernel kernel;
	
	cl_mem tex;
	cl_mem cam;
	cl_mem tri;
	
	cl_mem diffuse;
	
	unsigned long dim[4];
};

Engine_cl * engine_cl_init( size_t dim[4] );
void engine_cl_init_kernel( Engine_cl * ec, unsigned int gl_texture );
void engine_cl_render( Engine_cl * e, World * w );
