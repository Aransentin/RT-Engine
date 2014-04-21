#pragma once

typedef struct engine_struct Engine;

#include "engine_cl.h"
#include "engine_gl.h"
#include "world.h"

struct engine_struct
{
	Engine_gl * gl;
	Engine_cl * cl;
	World * world;
};

Engine * engine_init( size_t dim[4] );
void engine_render( Engine * e );
