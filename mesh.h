#pragma once

typedef struct mesh_struct Mesh;
struct mesh_struct
{
	float * v;
	unsigned int nv;
};

Mesh * mesh_load_obj( const char * file );
