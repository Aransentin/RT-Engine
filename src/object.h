typedef struct object_struct Object;

#include "mesh.h"

struct object_struct
{
	Mesh * mesh;
	unsigned int buffer_offset;
	char padding[4];
};

Object * object_new( void );
