#include "world.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

World * world_new( void )
{
	World * w = calloc( 1, sizeof(World) );
	w->texture = texture_load_all();
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

ObjectGPU * world_object_serialize( World * w )
{
	static ObjectGPU * objects = NULL;
	
	objects = realloc( objects, w->n_obj*sizeof(ObjectGPU) );
	
	for( unsigned int i=0; i<w->n_obj; i++)
	{
		memcpy( objects[i].bba.s, w->objects[i]->mesh->bb[0], sizeof( cl_float )*3 );
		memcpy( objects[i].bbb.s, w->objects[i]->mesh->bb[1], sizeof( cl_float )*3 );
		
		objects[i].bba.s[0] -= 0.0f;
		objects[i].bbb.s[0] += 0.0f;
		
		objects[i].nTri = w->objects[i]->mesh->nv/3;
		objects[i].offset = (cl_int)w->objects[i]->buffer_offset;
	}
	return objects;
}
