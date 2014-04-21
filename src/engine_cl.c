#include "engine_cl.h"
#include "error_cl.h"
#include "file.h"
#include "rtmath.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <GL/glx.h>

Engine_cl * engine_cl_init( size_t dim[4] )
{
	Engine_cl * ec = calloc( 1, sizeof(Engine_cl) );
	memcpy( ec->dim, dim, sizeof(size_t)*4 );
	
	cl_int err = 0;
	
	/*Get available platforms*/
	cl_platform_id platforms[8];
	cl_uint n_platforms = 0;
	err = clGetPlatformIDs( 8, platforms, &n_platforms );
	error_cl( __LINE__, err );
	
	/*Print platform info*/
	if ( n_platforms > 0 )
	{
		printf( "Detected platforms:\n" );
		for( unsigned int i=0; i< n_platforms; i++ )
		{
			char strings[4][2048] = {{ 0 }};
			clGetPlatformInfo( platforms[i], CL_PLATFORM_PROFILE, 2048, strings[0], NULL );
			clGetPlatformInfo( platforms[i], CL_PLATFORM_VERSION, 2048, strings[1], NULL );
			clGetPlatformInfo( platforms[i], CL_PLATFORM_NAME, 2048, strings[2], NULL );
			clGetPlatformInfo( platforms[i], CL_PLATFORM_VENDOR, 2048, strings[3], NULL );
			
			if ( i==0 )
				printf( "*0:\n" );
			else
				printf( " %i:\n", i );
			printf( " CL_PLATFORM_PROFILE:    %s\n CL_PLATFORM_VERSION:    %s\n CL_PLATFORM_NAME:       %s\n CL_PLATFORM_VENDOR:     %s\n", strings[0], strings[1], strings[2], strings[3] );
		}
	}
	else
	{
		printf( "Error: No OpenCL platforms detected\n" );
		exit( 1 );
	}
	printf( "\n" );
	
	ec->platform = platforms[0];
	
	/*Get available devices*/
	cl_device_id devices[8];
	cl_uint n_devices = 0;
	
	err = clGetDeviceIDs( ec->platform, CL_DEVICE_TYPE_ALL, 8, devices, &n_devices );
	error_cl( __LINE__, err );
	
	/*Print device info*/
	if ( n_devices > 0 )
	{
		printf( "Detected devices:\n" );
		for( unsigned int i=0; i< n_platforms; i++ )
		{
			char strings[3][1024] = {{ 0 }};
			clGetDeviceInfo( devices[i], CL_DEVICE_NAME, 1024, &strings[0], NULL );
			clGetDeviceInfo( devices[i], CL_DEVICE_VERSION, 1024, &strings[1], NULL );
			clGetDeviceInfo( devices[i], CL_DRIVER_VERSION, 1024, &strings[2], NULL );
			if ( i==0 )
				printf( "*0:\n" );
			else
				printf( " %i:\n", i );
			printf( " CL_DEVICE_NAME:    %s\n CL_DEVICE_VERSION: %s\n CL_DRIVER_VERSION: %s\n", strings[0], strings[1], strings[2] );
		}
	}
	else
	{
		printf( "Error: No OpenCL devices detected\n" );
		exit( 1 );
	}
	printf( "\n" );
	
	ec->device = devices[0];
	
	/*Test if context sharing is available*/
	size_t extensions_size;
	char * extensions;
	clGetDeviceInfo( ec->device, CL_DEVICE_EXTENSIONS, 0, NULL, &extensions_size );
	extensions = malloc( extensions_size );
	err = clGetDeviceInfo( ec->device, CL_DEVICE_EXTENSIONS, extensions_size, extensions, NULL );
	error_cl( __LINE__, err );
	
	if ( !strstr( extensions, "cl_khr_gl_sharing") )
	{
		printf( "Error: OpenCL \"cl_khr_gl_sharing\" support not found!\n" );
		exit( 1 );
	}
	free( extensions );
	
	/*OpenCL context properties*/
	cl_context_properties properties[] = 
	{
		CL_GL_CONTEXT_KHR, (cl_context_properties)(glXGetCurrentContext()),
		CL_GLX_DISPLAY_KHR, (cl_context_properties)(glXGetCurrentDisplay()),
		CL_CONTEXT_PLATFORM, (cl_context_properties)(ec->platform),
		0
	};
	
	/*Create a context and queue*/
	ec->context = clCreateContext( properties, 1, &ec->device, NULL, NULL, &err );
	error_cl( __LINE__, err );
	ec->queue = clCreateCommandQueue( ec->context, ec->device, 0, &err );
	error_cl( __LINE__, err );

	/*Load program*/
	char * program_src = readFile( "assets/render.cl" );
	ec->program = clCreateProgramWithSource( ec->context, 1, (const char**)&program_src, NULL, &err );
	error_cl( __LINE__, err );
	free( program_src );
	
	/*Make build options string, add ricer optimizations*/
	/*Additional optimizations you can try:  -cl-denorms-are-zero */
	char opt[2048] = {0};
	snprintf( opt, 2048, "-D DIM_X=%lu.0f -D DIM_Y=%lu.0f -cl-single-precision-constant -cl-strict-aliasing -cl-mad-enable -cl-no-signed-zeros -cl-fast-relaxed-math", dim[2], dim[3] );
	
	/*Compile program into a kernel*/ 
	err = clBuildProgram( ec->program, 1, &ec->device, opt, NULL, NULL );
	if ( err )
	{
		char buf[1024*4] = {0};
		clGetProgramBuildInfo( ec->program, ec->device, CL_PROGRAM_BUILD_LOG, 1024*4, buf, NULL );
		printf( "OpenCL kernel compile error %i: - %s\n", err, buf );
	}
	
	ec->kernel = clCreateKernel( ec->program, "core", &err );
	error_cl( __LINE__, err );
	
	return ec;
}

void engine_cl_init_kernel( Engine_cl * ec, unsigned int gl_texture )
{
	cl_int err = 0;
	
	/*Create image from OpenGL texture*/
	ec->tex = clCreateFromGLTexture2D( ec->context, CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, gl_texture, &err );
	error_cl( __LINE__, err );
	
	/*Create a memory buffer for the camera*/
	ec->cam = clCreateBuffer( ec->context, CL_MEM_READ_ONLY, sizeof(cl_float)*4*4, NULL, &err );
	error_cl( __LINE__, err );
	
	/*Set kernel arguments*/
	err = clSetKernelArg( ec->kernel, 0, sizeof( cl_mem ), &ec->tex );
	error_cl( __LINE__, err );
	
	err = clSetKernelArg( ec->kernel, 1, sizeof( cl_mem ), &ec->cam );
	error_cl( __LINE__, err );
}

void engine_cl_render( Engine_cl * e, World * w )
{
	cl_int err = 0;
	
	#include <GLFW/glfw3.h>
	
	/*Set camera values*/
	cl_float cam[4*4] = {0};
	float pos[3] = { 6.0f*cosf((float)glfwGetTime()), 6.0f*sinf((float)glfwGetTime()), 2.0f };
	float dir[3] = { cosf((float)glfwGetTime() + 3.14159f ), sinf((float)glfwGetTime()+ 3.14159f ), -0.4f };
	float nor[3] = { 0.0, 0.0, 1.0 };
	
	rtmath_lookTo( cam, pos, dir, nor, 0.7f, (float)e->dim[0]/(float)e->dim[1] );
	
	err = clEnqueueWriteBuffer( e->queue, e->cam, CL_FALSE, 0, sizeof(cl_float)*4*4, cam, 0, NULL, NULL );
	error_cl( __LINE__, err );
	
	/*Upload triangle data, if neccesary*/
	if ( w->dirty )
	{
		unsigned int nvert = 0;
		float * buffer = world_vertex_serialize( w, &nvert );
		
		/*Destroy the old buffer, if it exists*/
		if ( e->tri != NULL )
		{
			err = clReleaseMemObject( e->tri );
			error_cl( __LINE__, err );
		}
		
		/*Create a new memory buffer for the triangles*/
		e->tri = clCreateBuffer( e->context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float)*8*nvert, buffer, &err );
		error_cl( __LINE__, err );
		
		/*Set the kernel argument*/
		err = clSetKernelArg( e->kernel, 2, sizeof( cl_mem ), &e->tri );
		error_cl( __LINE__, err );

		w->dirty = 0;
	}
	
	/*Acquire the OpenGL texture*/
	err = clEnqueueAcquireGLObjects( e->queue, 1, &e->tex, 0, NULL, NULL );
	error_cl( __LINE__, err );
	
	/*Dispatch work to the kernel*/
	err = clEnqueueNDRangeKernel( e->queue, e->kernel, 2, NULL, &(e->dim[2]), NULL, 0, NULL, NULL );
	error_cl( __LINE__, err );
	
	/*Release the OpenGL texture*/
	err = clEnqueueReleaseGLObjects( e->queue, 1, &e->tex, 0, NULL, NULL );
	error_cl( __LINE__, err );
	
	/*Finish tasks*/
	clFinish( e->queue );
}
