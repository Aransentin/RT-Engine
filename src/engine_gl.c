#include "engine_gl.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

Engine_gl * engine_gl_init( size_t dim[4] )
{
	Engine_gl * eg = calloc( 1, sizeof(Engine_gl) );
	memcpy( eg->dim, dim, sizeof(size_t)*4 );
	
	/*Init GLFW library*/
	if ( !glfwInit() )
	{
		printf( "Error: GLFW failed to start!\n" );
		exit( 0 );
	}
	
	/*We aren't using any depreceated functionality.*/
	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
	glfwWindowHint( GLFW_RESIZABLE, 0 ); 
	glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
	glfwWindowHint( GLFW_STENCIL_BITS, 0 );
	
	/*Create a window*/
	eg->window = glfwCreateWindow( (int)eg->dim[0], (int)eg->dim[1], "Main window", NULL, NULL );
	
	/*Setup OpenGL state*/
	glfwMakeContextCurrent( eg->window );
	glfwSwapInterval( 1 );
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_CULL_FACE );

	/*Generate main window buffer*/
	glGenVertexArrays( 1, &eg->VAO );
	glGenBuffers( 1, &eg->VBO );
	
	/*Window vertex data*/
	/*TODO: calculate UV pos*/
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
	glBindVertexArray( eg->VAO );
	glBindBuffer( GL_ARRAY_BUFFER, eg->VBO );
	glBufferData( GL_ARRAY_BUFFER, sizeof(vdata), vdata, GL_STATIC_DRAW );
	
	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, 0 );
	glEnableVertexAttribArray( 1 );
	glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, (void*)(sizeof(float)*2) );
	
	/*Generate shader*/
	eg->shader = shader_init();
	
	return eg;
}

void engine_gl_init_texture( Engine_gl * e )
{
	glActiveTexture( GL_TEXTURE0 );
	glGenTextures( 1, &e->TEX );
	glBindTexture( GL_TEXTURE_2D, e->TEX );
	
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	
	if ( e->dim[0] > e->dim[2] && e->dim[1] > e->dim[3] )
	{
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	}
	else
	{
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	}
	char * ebuf = calloc( 1, e->dim[2]*e->dim[3]*4 );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, (int)e->dim[2], (int)e->dim[3], 0, GL_RGBA, GL_UNSIGNED_BYTE, ebuf );
	glBindTexture( GL_TEXTURE_2D, 0 );
	free( ebuf );
}

void engine_gl_error_test( void )
{
	GLenum error = glGetError();
	if ( error != GL_NO_ERROR )
		printf( "OpenGL error: %i\n", error );
}

void engine_gl_render( Engine_gl * e )
{
	engine_gl_error_test();
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	/*Bind texture that OpenCL rendered into*/
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, e->TEX );
	
	/*Bind window vertices*/
	glBindVertexArray( e->VAO );
	
	/*Render, flush*/
	glDrawArrays( GL_TRIANGLES, 0, 6 );
	glfwSwapBuffers( e->window );
	
	glBindTexture( GL_TEXTURE_2D, 0 );
	glfwPollEvents();
	
	glFinish();
}
