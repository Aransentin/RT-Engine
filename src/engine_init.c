#include "engine_init.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "error.h"
#include "file.h"
#include "texture.h"

#ifdef __unix__
	#include <GL/glx.h>
#elif _WIN32
	#include <windows.h>
#endif

void engine_init( Engine * e )
{
	/*==== OpenGL ====*/
	
	if ( !glfwInit() )
		error_die( "GLFW failed to start" );
	
	/*We aren't using any depreceated functionality.*/
	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
	glfwWindowHint( GLFW_RESIZABLE, 0 ); 
	glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
	glfwWindowHint( GLFW_STENCIL_BITS, 0 );
	
	/*Create a window*/
	e->window = glfwCreateWindow( (int)e->dim_window[0], (int)e->dim_window[1], "Main window", NULL, NULL );
	
	/*Setup OpenGL state*/
	glfwMakeContextCurrent( e->window );
	glfwSwapInterval( 1 );
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_CULL_FACE );
	
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	
	/*Generate main window buffer*/
	glGenVertexArrays( 1, &e->VAO );
	glGenBuffers( 1, &e->VBO );
	
	/*Window vertex data*/
	float vdata[4*6] = 
	{
		-1.0f, -1.0f, 0.0f, 0.0f,
		 1.0f, -1.0f, 1.0f, 0.0f,
		 1.0f,  1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f,
		 1.0f,  1.0f, 1.0f, 1.0f,
		-1.0f,  1.0f, 0.0f, 1.0f
	};
	
	/*Setup window buffers, upload vertex data*/
	glBindVertexArray( e->VAO );
	glBindBuffer( GL_ARRAY_BUFFER, e->VBO );
	glBufferData( GL_ARRAY_BUFFER, sizeof(vdata), vdata, GL_STATIC_DRAW );
	
	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, 0 );
	glEnableVertexAttribArray( 1 );
	glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, (void*)(sizeof(float)*2) );
	
	/*Generate shader*/
	e->shader = shader_init();
	
	/*Main output texture*/
	glActiveTexture( GL_TEXTURE0 );
	glGenTextures( 1, &e->output_texture_GL );
	glBindTexture( GL_TEXTURE_2D, e->output_texture_GL );
	
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	
	if ( e->dim_window[0] > e->dim_internal[0] && e->dim_window[1] > e->dim_internal[1] )
	{
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	}
	else
	{
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	}
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, (int)e->dim_internal[0], (int)e->dim_internal[1], 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
	glBindTexture( GL_TEXTURE_2D, 0 );

	/*==== OpenCL ====*/
	
	cl_int err = 0;
	
	/*Get platform & device*/
	err = clGetPlatformIDs( 1, &e->platform, NULL );
	error_cl( __LINE__, err );
	
	err = clGetDeviceIDs( e->platform, CL_DEVICE_TYPE_ALL, 1, &e->device, NULL );
	error_cl( __LINE__, err );
	
	/*Get device information*/
	char strings[3][1024] = {{ 0 }};
	clGetDeviceInfo( e->device, CL_DEVICE_NAME, 1024, &strings[0], NULL );
	clGetDeviceInfo( e->device, CL_DEVICE_VERSION, 1024, &strings[1], NULL );
	clGetDeviceInfo( e->device, CL_DRIVER_VERSION, 1024, &strings[2], NULL );
	
	printf( "Using OpenCL device '%s' - %s (%s)\n", strings[0], strings[1], strings[2] );
	
	/*Test if context sharing is available*/
	size_t extensions_size;
	char * extensions;
	clGetDeviceInfo( e->device, CL_DEVICE_EXTENSIONS, 0, NULL, &extensions_size );
	extensions = malloc( extensions_size );
	err = clGetDeviceInfo( e->device, CL_DEVICE_EXTENSIONS, extensions_size, extensions, NULL );
	error_cl( __LINE__, err );
	
	if ( !strstr( extensions, "cl_khr_gl_sharing") )
		printf( "Warning: OpenCL \"cl_khr_gl_sharing\" support not found - falling back to manual texture transfer, which is quite slow.\n" );
	else
		e->gl_sharing_support = 1;
	free( extensions );
	
	/*OpenCL context properties*/
	#ifdef __unix__ 
		cl_context_properties properties[] = 
		{
			CL_GL_CONTEXT_KHR, (cl_context_properties)(glXGetCurrentContext()),
			CL_GLX_DISPLAY_KHR, (cl_context_properties)(glXGetCurrentDisplay()),
			CL_CONTEXT_PLATFORM, (cl_context_properties)(e->platform),
			0
		};
	#elif _WIN32
		cl_context_properties properties[] = 
		{
			CL_GL_CONTEXT_KHR, (cl_context_properties) wglGetCurrentContext(), 
			CL_WGL_HDC_KHR, (cl_context_properties) wglGetCurrentDC(),
			CL_CONTEXT_PLATFORM, (cl_context_properties)(e->platform),
			0
		};
	#else
		#error Get a proper OS.
	#endif
	
	/*Create a context and queue*/
	e->context = clCreateContext( properties, 1, &e->device, NULL, NULL, &err );
	error_cl( __LINE__, err );
	
	e->queue = clCreateCommandQueue( e->context, e->device, 0, &err );
	error_cl( __LINE__, err );
	
	/*Vector buffer*/
	e->vec_buf = clCreateBuffer( e->context, CL_MEM_READ_WRITE, sizeof(cl_float)*8*e->dim_internal[0]*e->dim_internal[1], NULL, &err );
	error_cl( __LINE__, err );
	
	/*Sort buffer*/
	e->sort_buf = clCreateBuffer( e->context, CL_MEM_READ_WRITE, sizeof(cl_int)*4*e->dim_internal[0]*e->dim_internal[1], NULL, &err );
	error_cl( __LINE__, err );
	
	/*Trace buffer*/
	e->trace_buf = clCreateBuffer( e->context, CL_MEM_READ_WRITE, sizeof(cl_float)*16*e->dim_internal[0]*e->dim_internal[1], NULL, &err );
	error_cl( __LINE__, err );
	
	/*Colour buffer*/
	e->colour_buf = clCreateBuffer( e->context, CL_MEM_READ_WRITE, sizeof(cl_float)*4*e->dim_internal[0]*e->dim_internal[1], NULL, &err );
	error_cl( __LINE__, err );

	/*Light buffer*/
	e->light_buf = clCreateBuffer( e->context, CL_MEM_READ_WRITE, sizeof(cl_float)*4*e->dim_internal[0]*e->dim_internal[1], NULL, &err );
	error_cl( __LINE__, err );

	/*Load texture*/
	texture_load( &e->texture_data, e->texture_dim );
	cl_image_format texture_format = { .image_channel_order=CL_RGBA, .image_channel_data_type=CL_UNORM_INT8 };
	e->dif_ref_texture = clCreateImage2D( e->context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &texture_format, e->texture_dim[0], e->texture_dim[1], 0, e->texture_data, &err );
	error_cl( __LINE__, err );
	
	/*Load kernels*/
	char * ks[8];
	ks[K_GEN] = readFile( "kernels/raygen.cl" );
	ks[K_SORT] = readFile( "kernels/sort.cl" );
	ks[K_COMPOSE] = readFile( "kernels/compose.cl" );
	ks[K_TRACE] = readFile( "kernels/trace.cl" );
	ks[K_INTEROP] = readFile( "kernels/interop.cl" );
	ks[K_LIGHT] = readFile( "kernels/light.cl" );
	ks[K_LIGHTSORT] = readFile( "kernels/light_sort.cl" );
	
	/*Create program*/
	e->program = clCreateProgramWithSource( e->context, 7, (const char **)ks, NULL, &err );
	error_cl( __LINE__, err );
	
	/*Make build options string, add ricer optimizations*/
	char opt[2048] = {0};
	snprintf( opt, 2048, "-D DIM_X=%lu -D DIM_Y=%lu -cl-single-precision-constant -cl-strict-aliasing -cl-mad-enable -cl-no-signed-zeros -cl-fast-relaxed-math  -cl-denorms-are-zero", e->dim_internal[0], e->dim_internal[1] );
	err = clBuildProgram( e->program, 1, &e->device, opt, NULL, NULL );
	if ( err )
	{
		char buf[1024*4] = {0};
		clGetProgramBuildInfo( e->program, e->device, CL_PROGRAM_BUILD_LOG, 1024*4, buf, NULL );
		printf( "OpenCL kernel compile error %i: - %s\n", err, buf );
	}
	
	for( int i=0; i<7; i++ )
		free( ks[i] );
	
	/*Ray generation kernel*/
	e->kernels[K_GEN] = clCreateKernel( e->program, "generate_rays", &err );
	error_cl( __LINE__, err );
	err = clSetKernelArg( e->kernels[K_GEN], 4, sizeof( cl_mem ), &e->vec_buf );
	error_cl( __LINE__, err );
	
	/*Sort kernel*/
	e->kernels[K_SORT] = clCreateKernel( e->program, "sort", &err );
	err = clSetKernelArg( e->kernels[K_SORT], 0, sizeof( cl_mem ), &e->vec_buf );
	error_cl( __LINE__, err );
	err = clSetKernelArg( e->kernels[K_SORT], 3, sizeof( cl_mem ), &e->sort_buf );
	error_cl( __LINE__, err );
	
	/*Trace kernel*/
	e->kernels[K_TRACE] = clCreateKernel( e->program, "trace", &err );
	err = clSetKernelArg( e->kernels[K_TRACE], 0, sizeof( cl_mem ), &e->vec_buf );
	error_cl( __LINE__, err );
	err = clSetKernelArg( e->kernels[K_TRACE], 1, sizeof( cl_mem ), &e->sort_buf );
	error_cl( __LINE__, err );
	err = clSetKernelArg( e->kernels[K_TRACE], 4, sizeof( cl_mem ), &e->trace_buf );
	error_cl( __LINE__, err );
	
	/*Compose kernel*/
	e->kernels[K_COMPOSE] = clCreateKernel( e->program, "compose", &err );
	error_cl( __LINE__, err );
	err = clSetKernelArg( e->kernels[K_COMPOSE], 0, sizeof( cl_mem ), &e->trace_buf );
	error_cl( __LINE__, err );
	err = clSetKernelArg( e->kernels[K_COMPOSE], 1, sizeof( cl_mem ), &e->dif_ref_texture );
	error_cl( __LINE__, err );
	err = clSetKernelArg( e->kernels[K_COMPOSE], 2, sizeof( cl_mem ), &e->colour_buf );
	error_cl( __LINE__, err );
	err = clSetKernelArg( e->kernels[K_COMPOSE], 3, sizeof( cl_mem ), &e->vec_buf );
	error_cl( __LINE__, err );
	err = clSetKernelArg( e->kernels[K_COMPOSE], 4, sizeof( cl_mem ), &e->light_buf );
	error_cl( __LINE__, err );
	
	/*Light kernel*/
	e->kernels[K_LIGHT] = clCreateKernel( e->program, "light", &err );
	error_cl( __LINE__, err );
	err = clSetKernelArg( e->kernels[K_LIGHT], 0, sizeof( cl_mem ), &e->trace_buf );
	error_cl( __LINE__, err );
	err = clSetKernelArg( e->kernels[K_LIGHT], 2, sizeof( cl_mem ), &e->light_buf );
	error_cl( __LINE__, err );
	err = clSetKernelArg( e->kernels[K_LIGHT], 3, sizeof( cl_mem ), &e->sort_buf );
	error_cl( __LINE__, err );
	
	/*Light sort kernel*/
	e->kernels[K_LIGHTSORT] = clCreateKernel( e->program, "lightsort", &err );
	err = clSetKernelArg( e->kernels[K_LIGHTSORT], 0, sizeof( cl_mem ), &e->trace_buf );
	error_cl( __LINE__, err );
	err = clSetKernelArg( e->kernels[K_LIGHTSORT], 4, sizeof( cl_mem ), &e->sort_buf );
	error_cl( __LINE__, err );
	
	/*Interop kernel*/
	e->kernels[K_INTEROP] = clCreateKernel( e->program, "interop", &err );
	error_cl( __LINE__, err );
	err = clSetKernelArg( e->kernels[K_INTEROP], 0, sizeof( cl_mem ), &e->colour_buf );
	
	
	
	/*Output texture*/
	if ( e->gl_sharing_support == 1 )
	{
		e->output_texture_CL = clCreateFromGLTexture2D( e->context, CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, e->output_texture_GL, &err );
		error_cl( __LINE__, err );
	}
	else
	{
		cl_image_format format = { .image_channel_order=CL_RGBA, .image_channel_data_type=CL_UNORM_INT8 };
		e->output_texture_CL = clCreateImage2D( e->context, CL_MEM_WRITE_ONLY, &format, e->dim_internal[0], e->dim_internal[1], 0, NULL, &err );
		error_cl( __LINE__, err );
	}
	
	err = clSetKernelArg( e->kernels[K_INTEROP], 1, sizeof( cl_mem ), &e->output_texture_CL );
	error_cl( __LINE__, err );
}
