#pragma once

typedef struct world_struct World;

#include "object.h"
#include "mesh.h"

struct world_struct
{
	Object ** objects;
	unsigned int obj_n;
	char dirty;
};

World * world_new( void );
float * world_serialize_bb( World * w );
float * world_serialize_vert( World * w );
int * world_serialize_obj( World * w );
unsigned int world_geo_n( World * w );
unsigned int world_vert_n( World * w );
void world_recalculate_dirty( World * w );

Object * object_new( World * w );
void object_attach_mesh( World * w, Object * o, Mesh * m );
void object_vert_transform( Object * o );
