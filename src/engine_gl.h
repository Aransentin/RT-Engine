#pragma once

typedef struct engine_gl_struct Engine_gl;

#include "glcorearb.h"
#include <GLFW/glfw3.h>
#include "shader_gl.h"
#include "world.h"

struct engine_gl_struct
{
	size_t dim[4];
	GLFWwindow * window;
	Shader * shader;
	GLuint VAO;
	GLuint VBO;
	GLuint TEX;
	char padding[4];
};

Engine_gl * engine_gl_init( size_t dim[4] );
void engine_gl_init_texture( Engine_gl * e );
void engine_gl_render( Engine_gl * e, World * w );
void engine_gl_error_test( void );
