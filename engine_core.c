#include "engine_core.h"

#include <stdlib.h>
#include <stdio.h>

Engine * engine_init( size_t dim[4] )
{
	Engine * e = calloc( 1, sizeof(Engine) );
	
	e->gl = engine_gl_init( dim );
	e->cl = engine_cl_init( dim );
	
	engine_gl_init_texture( e->gl );
	engine_cl_init_kernel( e->cl, e->gl->TEX );
	
	return e;
}

void engine_render( Engine * e )
{
	engine_cl_render( e->cl );
	engine_gl_render( e->gl );
}
