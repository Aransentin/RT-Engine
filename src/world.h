#pragma once

#include "mesh.h"
#include "object.h"

typedef struct world_struct World;

struct world_struct
{
	Object ** objects;
	unsigned int n_obj;
	char dirty;
	char padding[3];
};

World * world_new( void );
void world_addObject( World * w, Object * o );
float * world_vertex_serialize( World * w, unsigned int * nv_out );
