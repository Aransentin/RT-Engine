#include "engine.h"
#include "engine_init.h"
#include "error.h"
#include "rt_math.h"

#include <stdlib.h>

Engine * engine_new( unsigned int w, unsigned int h, unsigned int iw, unsigned int ih )
{
	Engine * e = calloc( 1, sizeof(Engine) );
	
	/*Setup things needed for the engine*/
	e->dim_window[0] = w;
	e->dim_window[1] = h;
	e->dim_internal[0] = iw;
	e->dim_internal[1] = ih;
	
	engine_init( e );
	
	return e;
}

void engine_render( Engine * e )
{
	world_recalculate_dirty( e->world );
	
	cl_int err = 0;
	cl_int numObj = (cl_int)world_geo_n( e->world );
	cl_int numVert = (cl_int)world_vert_n( e->world );
	
	/*Zero old buffers*/
	static char * empty_buf = NULL;
	if ( empty_buf == NULL )
		empty_buf = calloc( 1, e->dim_internal[0]*e->dim_internal[1]*4*sizeof(float) );
	
	err = clEnqueueWriteBuffer( e->queue, e->colour_buf, CL_TRUE, 0, e->dim_internal[0]*e->dim_internal[1]*4*sizeof(float), empty_buf, 0, NULL, NULL );
	error_cl( __LINE__, err );
	err = clEnqueueWriteBuffer( e->queue, e->light_buf, CL_TRUE, 0, e->dim_internal[0]*e->dim_internal[1]*4*sizeof(float), empty_buf, 0, NULL, NULL );
	error_cl( __LINE__, err );
	
	/*Recalculate buffers,if needed*/
	if ( e->world->dirty )
	{
		if ( e->objects_bb )
		{
			err = clReleaseMemObject( e->objects_bb );
			error_cl( __LINE__, err );
		}
		if ( e->vertices )
		{
			err = clReleaseMemObject( e->vertices );
			error_cl( __LINE__, err );
		}
		if ( e->objects )
		{
			err = clReleaseMemObject( e->objects );
			error_cl( __LINE__, err );
		}
		
		e->objects_bb = clCreateBuffer( e->context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_float)*8*(unsigned)numObj, world_serialize_bb( e->world ), &err );
		error_cl( __LINE__, err );
		
		e->vertices = clCreateBuffer( e->context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_float)*8*(unsigned)numVert, world_serialize_vert( e->world ), &err );
		error_cl( __LINE__, err );
		
		e->objects = clCreateBuffer( e->context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_int)*2*(unsigned)numObj, world_serialize_obj( e->world ), &err );
		error_cl( __LINE__, err );
		
		e->world->dirty = 0;
		
		/*Set kernel args for the newly created buffers*/
		err = clSetKernelArg( e->kernels[K_SORT], 1, sizeof( cl_mem ), &e->objects_bb );
		error_cl( __LINE__, err );
		
		err = clSetKernelArg( e->kernels[K_SORT], 2, sizeof( cl_int ), &numObj );
		error_cl( __LINE__, err );
		
		err = clSetKernelArg( e->kernels[K_LIGHTSORT], 1, sizeof( cl_mem ), &e->objects_bb );
		error_cl( __LINE__, err );
		
		err = clSetKernelArg( e->kernels[K_LIGHTSORT], 2, sizeof( cl_int ), &numObj );
		error_cl( __LINE__, err );
		
		err = clSetKernelArg( e->kernels[K_TRACE], 2, sizeof( cl_mem ), &e->objects );
		error_cl( __LINE__, err );
		
		err = clSetKernelArg( e->kernels[K_TRACE], 3, sizeof( cl_mem ), &e->vertices );
		error_cl( __LINE__, err );
		
		err = clSetKernelArg( e->kernels[K_LIGHT], 4, sizeof( cl_mem ), &e->objects );
		error_cl( __LINE__, err );
		
		err = clSetKernelArg( e->kernels[K_LIGHT], 5, sizeof( cl_mem ), &e->vertices );
		error_cl( __LINE__, err );
	}
	
	/*Ray generation*/
	float time = (float)glfwGetTime();
	//float time = 3.441f;
	cl_float cambuf[16] = { 0.0f };
	float pos[3] = { 10.0f*cosf(time), 10.0f*sinf(time), 4.0f };
	float dir[3] = { cosf(time + 3.14159f ), sinf(time + 3.14159f ), -0.6f };
	float nor[3] = { 0.0f, 0.0f, 1.0f };
	
	rtmath_lookTo( cambuf, pos, dir, nor, 1.0f, (float)e->dim_window[0]/(float)e->dim_window[1] );
	
	err = clSetKernelArg( e->kernels[K_GEN], 0, sizeof( cl_float )*4, &cambuf[0] );
	error_cl( __LINE__, err );
	err = clSetKernelArg( e->kernels[K_GEN], 1, sizeof( cl_float )*4, &cambuf[4] );
	error_cl( __LINE__, err );
	err = clSetKernelArg( e->kernels[K_GEN], 2, sizeof( cl_float )*4, &cambuf[8] );
	error_cl( __LINE__, err );
	err = clSetKernelArg( e->kernels[K_GEN], 3, sizeof( cl_float )*4, &cambuf[12] );
	error_cl( __LINE__, err );
	
	err = clEnqueueNDRangeKernel( e->queue, e->kernels[K_GEN], 2, NULL, e->dim_internal, NULL, 0, NULL, NULL );
	error_cl( __LINE__, err );

	for( int i=0; i<3; i++ )
	{
		/*Sort objects*/
		err = clEnqueueNDRangeKernel( e->queue, e->kernels[K_SORT], 2, NULL, e->dim_internal, NULL, 0, NULL, NULL );
		error_cl( __LINE__, err );
		
		/*Raytrace*/
		err = clEnqueueNDRangeKernel( e->queue, e->kernels[K_TRACE], 2, NULL, e->dim_internal, NULL, 0, NULL, NULL );
		error_cl( __LINE__, err );

		/*Set light position*/
		float lightp[4] = { -4.0f, -4.0f, 4.0f, 0.0f };
		err = clSetKernelArg( e->kernels[K_LIGHT], 1, sizeof( cl_float )*4, lightp );
		error_cl( __LINE__, err );
		err = clSetKernelArg( e->kernels[K_LIGHTSORT], 3, sizeof( cl_float )*4, lightp );
		error_cl( __LINE__, err );
		
		/*Sort light*/
		
		err = clEnqueueNDRangeKernel( e->queue, e->kernels[K_LIGHTSORT], 2, NULL, e->dim_internal, NULL, 0, NULL, NULL );
		error_cl( __LINE__, err );
		
		/*Calculate light*/
		err = clEnqueueNDRangeKernel( e->queue, e->kernels[K_LIGHT], 2, NULL, e->dim_internal, NULL, 0, NULL, NULL );
		error_cl( __LINE__, err );
		
		/*Compose image*/
		err = clEnqueueNDRangeKernel( e->queue, e->kernels[K_COMPOSE], 2, NULL, e->dim_internal, NULL, 0, NULL, NULL );
		error_cl( __LINE__, err );
	}
	
	/*Acquire the OpenGL texture*/
	if ( e->gl_sharing_support == 1 )
	{
		err = clEnqueueAcquireGLObjects( e->queue, 1, &e->output_texture_CL, 0, NULL, NULL );
		error_cl( __LINE__, err );
	}
	
	err = clEnqueueNDRangeKernel( e->queue, e->kernels[K_INTEROP], 2, NULL, e->dim_internal, NULL, 0, NULL, NULL );
	error_cl( __LINE__, err );
	
	/*Release the OpenGL texture*/
	if ( e->gl_sharing_support == 1 )
	{
		err = clEnqueueReleaseGLObjects( e->queue, 1, &e->output_texture_CL, 0, NULL, NULL );
		error_cl( __LINE__, err );
	}
	//clFinish( e->queue );
	
	/*=== OpenGL ===*/
	error_GL();
	
	//glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, e->output_texture_GL );
	glBindVertexArray( e->VAO );
	glDrawArrays( GL_TRIANGLES, 0, 6 );
	glfwSwapBuffers( e->window );
	
	glBindTexture( GL_TEXTURE_2D, 0 );
	glfwPollEvents();
	//glFinish();
}
