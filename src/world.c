#include "world.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

World * world_new( void )
{
	World * w = calloc( 1, sizeof(World) );
	return w;
}

void world_addObject( World * w, Object * o )
{
	w->objects = realloc( w->objects, sizeof( Object* )*(w->n_obj+1) );
	w->objects[w->n_obj] = o;
	w->n_obj++;
	w->dirty = 1;
}

float * world_vertex_serialize( World * w, unsigned int * nv_out )
{
	static float * vertices = NULL;
	
	unsigned int nv = 0;
	for( unsigned int i=0; i<w->n_obj; i++ )
	{
		Object * o = w->objects[i];
		if ( o->mesh )
		{
			o->buffer_offset = nv;
			nv += o->mesh->nv;
		}
	}
	
	vertices = realloc( vertices, nv*sizeof(float)*8 );
	
	if ( nv_out != NULL )
		*nv_out = nv;
	
	/*Todo: multiply all vertices with object matrix*/
	unsigned int vp = 0;
	for( unsigned int i=0; i<w->n_obj; i++ )
	{
		Object * o = w->objects[i];
		if ( o->mesh )
		{
			memcpy( &vertices[vp*8], o->mesh->v, (o->mesh->nv)*8*sizeof(float) );
			vp += o->mesh->nv;
		}
	
	}
	
	return vertices;
}
