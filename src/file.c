#include "file.h"

#include <stdlib.h>
#include <stdio.h>

char * readFile( const char * name )
{
	FILE * f = fopen( name, "rb" );
	if ( f )
	{
		fseek( f, 0, SEEK_END );
		size_t length = (size_t)ftell( f );
		rewind( f );
		char * buffer = calloc( 1, length+1 );
		size_t r = fread( buffer, 1, length, f );
		fclose( f );
		if ( r != 0 )
			return buffer;
		else
		{
			free( buffer );
			return NULL;
		}
	}
	return NULL;
}
