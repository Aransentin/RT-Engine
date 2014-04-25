#include "texture.h"

#include <stdlib.h>
#include <string.h>

#include "png.h"

static void texture_png_version_info( void )
{
    printf( "Compiled with libpng %s; using libpng %s.\n", PNG_LIBPNG_VER_STRING, png_libpng_ver );
}

Texture * texture_load_all( void )
{
	texture_png_version_info();
	Texture * t = calloc( 1, sizeof(Texture) );
	
	FILE *fp = fopen( "assets/texture.png", "rb" );
	if ( !fp )
	{
		printf( "Error: 'assets/texture.png' not found!\n" );
		exit( 1 );
	}
	
	unsigned char header[8];
	fread( header, 1, 8, fp );
	if ( png_sig_cmp(header, 0, 8) )
	{
		printf( "The file 'assets/texture.png' is not recognized as a PNG file.\n" );
		exit( 1 );
	}
	
	png_structp png_ptr;
	png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
	
	png_infop info_ptr;
	info_ptr = png_create_info_struct(png_ptr);
	
	setjmp( png_jmpbuf(png_ptr) );
	png_init_io( png_ptr, fp );
	png_set_sig_bytes( png_ptr, 8 );

	png_read_info( png_ptr, info_ptr );
	
	t->dim[0] = png_get_image_width( png_ptr, info_ptr );
	t->dim[1] = png_get_image_height( png_ptr, info_ptr );
	int color_type = png_get_color_type( png_ptr, info_ptr );
	png_read_update_info( png_ptr, info_ptr );

	if ( setjmp( png_jmpbuf(png_ptr) ) )
	{
		printf( "Error loading PNG\n" );
		exit( 1 );
	}
	png_bytep * row_pointers;
	row_pointers = (png_bytep*) malloc( sizeof(png_bytep) * t->dim[1] );
	
	for ( unsigned int y=0; y<t->dim[1]; y++ )
		row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));

        png_read_image( png_ptr, row_pointers );
	
        fclose( fp );
	
	t->data = malloc( t->dim[0]*t->dim[1]*4 );
	
	for ( unsigned int y=0; y<t->dim[1]; y++ )
	{
		for ( unsigned int x=0; x<t->dim[0]; x++ )
		{
			if (color_type == PNG_COLOR_TYPE_RGB )
			{
				t->data[ 4*(y*t->dim[0]+x)+0 ] = row_pointers[y][x*3+0];
				t->data[ 4*(y*t->dim[0]+x)+1 ] = row_pointers[y][x*3+1];
				t->data[ 4*(y*t->dim[0]+x)+2 ] = row_pointers[y][x*3+2];
				t->data[ 4*(y*t->dim[0]+x)+3 ] = 255;
			}
			else
			{
				t->data[ 4*(y*t->dim[0]+x)+0 ] = row_pointers[y][x*4+0];
				t->data[ 4*(y*t->dim[0]+x)+1 ] = row_pointers[y][x*4+1];
				t->data[ 4*(y*t->dim[0]+x)+2 ] = row_pointers[y][x*4+2];
				t->data[ 4*(y*t->dim[0]+x)+3 ] = row_pointers[y][x*4+3];
			}
		}
	}
	
	for ( unsigned int y=0; y<t->dim[1]; y++ )
		free( row_pointers[y] );
	free( row_pointers );
	
	return t;
}
