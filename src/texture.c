#include "texture.h"
#include "png.h"

#include <stdlib.h>

static void texture_png_version_info( void )
{
	printf( "Compiled with libpng %s; using libpng %s.\n", PNG_LIBPNG_VER_STRING, png_libpng_ver );
}

void texture_load( unsigned char ** data, unsigned long dim[2] )
{
	texture_png_version_info();
	
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
	
	dim[0] = png_get_image_width( png_ptr, info_ptr );
	dim[1] = png_get_image_height( png_ptr, info_ptr );
	int color_type = png_get_color_type( png_ptr, info_ptr );
	png_read_update_info( png_ptr, info_ptr );

	if ( setjmp( png_jmpbuf(png_ptr) ) )
	{
		printf( "Error loading PNG\n" );
		exit( 1 );
	}
	png_bytep * row_pointers;
	row_pointers = (png_bytep*) malloc( sizeof(png_bytep) * dim[1] );
	
	for ( unsigned int y=0; y<dim[1]; y++ )
		row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));

        png_read_image( png_ptr, row_pointers );
	
        fclose( fp );
	
	*data = malloc( dim[0]*dim[1]*4 );
	
	for ( unsigned int y=0; y<dim[1]; y++ )
	{
		for ( unsigned int x=0; x<dim[0]; x++ )
		{
			if (color_type == PNG_COLOR_TYPE_RGB )
			{
				(*data)[ 4*(y*dim[0]+x)+0 ] = row_pointers[y][x*3+0];
				(*data)[ 4*(y*dim[0]+x)+1 ] = row_pointers[y][x*3+1];
				(*data)[ 4*(y*dim[0]+x)+2 ] = row_pointers[y][x*3+2];
				(*data)[ 4*(y*dim[0]+x)+3 ] = 255;
			}
			else
			{
				(*data)[ 4*(y*dim[0]+x)+0 ] = row_pointers[y][x*4+0];
				(*data)[ 4*(y*dim[0]+x)+1 ] = row_pointers[y][x*4+1];
				(*data)[ 4*(y*dim[0]+x)+2 ] = row_pointers[y][x*4+2];
				(*data)[ 4*(y*dim[0]+x)+3 ] = row_pointers[y][x*4+3];
			}
		}
	}
	
	for ( unsigned int y=0; y<dim[1]; y++ )
		free( row_pointers[y] );
	free( row_pointers );
}
