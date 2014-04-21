#pragma once

typedef struct mesh_struct Mesh;
struct mesh_struct
{
	float * v;
	unsigned int nv;
	float bb[2][3];
	char padding[4];
};

Mesh * mesh_load_obj( const char * file );
