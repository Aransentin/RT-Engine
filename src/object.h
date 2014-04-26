#pragma once

typedef struct object_struct Object;

#include "mesh.h"

struct object_struct
{
	float mat[16];
	float bb[8];
	Mesh * mesh;
	
	/*Vertex data*/
	float * v;
	unsigned int vo;
	
	/*Flags*/
	char dynamic;
	char dirty;
};
