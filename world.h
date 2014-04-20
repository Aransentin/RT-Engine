#pragma once

typedef struct vertex_struct Vertex;

struct vertex_struct
{
	float v[8];
};

Vertex * world_testing( unsigned int * n_vert );
