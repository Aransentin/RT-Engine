#pragma once

typedef struct shader_struct Shader;

#include "glcorearb.h"
#include <GLFW/glfw3.h>

struct shader_struct
{
	GLuint program;
	GLuint shader[2];
};

Shader * shader_init( void );
