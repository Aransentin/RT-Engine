#pragma once

typedef struct engine_struct Engine;

#define K_GEN 0
#define K_SORT 1
#define K_TRACE 2
#define K_COMPOSE 3
#define K_LIGHT 4
#define K_LIGHTSORT 5
#define K_INTEROP 6

#include "glcorearb.h"
#include <GLFW/glfw3.h>
#include <CL/cl.h>
#include <CL/cl_gl.h>
#include "shader.h"
#include "world.h"

struct engine_struct
{
	unsigned long dim_internal[2];
	unsigned long dim_window[2];
	
	/*OpenGL-specific things*/
	GLFWwindow * window;
	Shader * shader;
	GLuint VAO;
	GLuint VBO;
	
	/*OpenCL-specific things*/
	cl_platform_id platform;
	cl_device_id device;
	cl_context context;
	cl_command_queue queue;
	cl_program program;
	cl_kernel kernels[8];
	
	/*Opencl world buffers*/
	cl_mem objects_bb;
	cl_mem vertices;
	cl_mem objects;
	
	/*Mesh texture*/
	cl_mem dif_ref_texture;
	unsigned char * texture_data;
	unsigned long texture_dim[2];
	
	/* OpenCL transfer buffers*/
	cl_mem vec_buf;
	cl_mem sort_buf;
	cl_mem trace_buf;
	cl_mem colour_buf;
	cl_mem light_buf;
	
	/*GL-CL interop*/
	int gl_sharing_support;
	cl_mem output_texture_CL;
	GLuint output_texture_GL;
	
	World * world;
};

Engine * engine_new( unsigned int w, unsigned int h, unsigned int iw, unsigned int ih );
void engine_render( Engine * e );
