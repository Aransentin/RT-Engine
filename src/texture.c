#include "texture.h"

#include <stdlib.h>
#include <string.h>

Texture * texture_load_placeholder( void )
{
	#include "texture_data.h"
	Texture * t = calloc( 1, sizeof(Texture) );
	
	t->dim[0] = width;
	t->dim[1] = height;
	
	t->data = malloc( 4*width*height );
	
	const char * data = header_data;
	for( unsigned int i=0; i<width*height; i++ )
	{
		unsigned int pixel[4] = {0};
		HEADER_PIXEL( data, pixel );
		t->data[i*4+0] = (unsigned char)pixel[0];
		t->data[i*4+1] = (unsigned char)pixel[1];
		t->data[i*4+2] = (unsigned char)pixel[2];
		t->data[i*4+3] = (unsigned char)pixel[3];
	}
	
	return t;
}
