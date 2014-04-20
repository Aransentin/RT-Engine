#pragma once

typedef struct engine_struct Engine;

#include "engine_cl.h"
#include "engine_gl.h"

struct engine_struct
{
	Engine_gl * gl;
	Engine_cl * cl;
};

Engine * engine_init( size_t dim[4] );
void engine_render( Engine * e );
