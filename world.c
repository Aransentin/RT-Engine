#include "world.h"
#include <stdlib.h>

float * world_testing( unsigned int * n_vert )
{
	static Mesh * m = NULL;
	
	if( !m )
		m = mesh_load_obj( "monkey.obj" );
	
	if ( n_vert != NULL )
		*n_vert = m->nv;
	
	return m->v;
}
