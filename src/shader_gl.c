#include "shader_gl.h"

#include <stdlib.h>
#include <stdio.h>
#include "file.h"

static void shader_test_compile( GLuint vs, GLuint fs )
{
	GLsizei result[2] = {0};
        char log[2][1024];

        glGetShaderInfoLog( vs, 1024, &result[0], log[0] );
        glGetShaderInfoLog( fs, 1024, &result[1], log[1] );
	
        if ( result[0] != 0 )
	{
                printf( "Error: Vertex shader: %s\n", log[0] );
	}
        if ( result[1] != 0 )
	{
                printf( "Error: Fragment shader: %s\n", log[1] );
	}
	if ( result[0] != 0 || result[1] != 0 )
	{
		exit( 1 );
	}
}

static void shader_test_link( GLuint program )
{
        GLsizei result = 0;
        char log[1024]={0};

        glGetProgramInfoLog( program, 1024, &result, log );

        if ( result != 0 )
	{
                printf( "Error: Shader program:%s\n", log );
		exit( 1 );
	}
}

Shader * shader_init( void )
{
	Shader * s = calloc( 1, sizeof(Shader) );
	
	/*Read shader source files*/
	char * vs = readFile( "assets/vert.glsl" );
	char * fs = readFile( "assets/frag.glsl" );
	
	/*Compile shaders*/
	s->shader[0] = glCreateShader( GL_VERTEX_SHADER );
	s->shader[1] = glCreateShader( GL_FRAGMENT_SHADER );
	glShaderSource( s->shader[0], 1, (const GLchar* const*)&vs, 0 );
	glShaderSource( s->shader[1], 1, (const GLchar* const*)&fs, 0 );
	glCompileShader( s->shader[0] );
	glCompileShader( s->shader[1] );
	shader_test_compile( s->shader[0], s->shader[1] );
	free( vs );
	free( fs );
	
	/*Create the shader program*/
	s->program = glCreateProgram();
	glAttachShader( s->program, s->shader[0] );
	glAttachShader( s->program, s->shader[1] );
	
	/*Bind locations*/
	glBindAttribLocation( s->program, 0, "vPos" );
	glBindAttribLocation( s->program, 1, "vUV" );
	glBindFragDataLocation( s->program, 0, "fCol" );
	
	/*Link shader*/
	glLinkProgram( s->program );
	shader_test_link( s->program );
	
	glUseProgram( s->program );
	glUniform1i( glGetUniformLocation( s->program, "mtex" ), 0 );
	
	return s;
}
