#include "world.h"
#include <stdlib.h>

#include "mesh.h"
/*static Triangle tris[] = { 
	{{
		-4.0f, -4.0f, 0.0f, 0.0f,
		4.0f, -4.0f, 0.0f, 0.0f,
		-4.0f, 4.0f, 0.0f, 0.0f,
	}},
	{{
		4.0f, 4.0f, 0.0f, 0.0f,
		4.0f, -4.0f, 0.0f, 0.0f,
		-4.0f, 4.0f, 0.0f, 0.0f,
	}},
};*/

Vertex * world_testing( unsigned int * n_vert )
{
	if ( n_vert != NULL )
		*n_vert = sizeof( vertices )/sizeof( Vertex );
	return vertices;
}
