#pragma once

typedef struct texture_struct Texture;

struct texture_struct
{
	unsigned char * data;
	unsigned int dim[2];
};

Texture * texture_load_placeholder( void );
