#include "mesh.h"
#include "file.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

Mesh * mesh_load_obj( const char * filename )
{
	char * buf = readFile( filename );
	if ( !buf )
	{
		printf( "Error: file %s not found.\n", filename );
		exit( 0 );
	}
	Mesh * m = calloc( 1, sizeof(Mesh) );
	
	unsigned int n_pos=0, n_uv=0, n_nor=0, n_f=0;
	
	/*Go through buffer, count vertices/uvs/normals/faces*/
	char linebuf[1024] = {0};
	char * bp = buf;
	while( bp[0] != 0 )
	{
		sscanf( bp, "%1023[^\n]", linebuf );

		if ( linebuf[0] == 'v' && linebuf[1] == ' ' )
			n_pos++;
		else if ( linebuf[0] == 'v' && linebuf[1] == 't' )
			n_uv++;
		else if ( linebuf[0] == 'v' && linebuf[1] == 'n' )
			n_nor++;
		else if ( linebuf[0] == 'f' )
		{
			n_f++;
		}
		bp += strlen( linebuf )+1;
		
	}
	
	/*Allocate buffers*/
	float * v_pos = malloc( sizeof(float)*n_pos*3 );
	float * v_uv = malloc( sizeof(float)*n_uv*2 );
	float * v_nor = malloc( sizeof(float)*n_nor*3 );
	unsigned int * f_ind = malloc( sizeof(unsigned int)*n_f*9 );
	m->v = malloc( sizeof(float)*n_f*3*8 );
	
	/*Go through buffer, transfer vertices/uvs/normals/faces*/
	unsigned int i_pos=0, i_uv=0, i_nor=0, i_f=0;
	bp = buf;
	while( bp[0] != 0 )
	{

		sscanf( bp, "%1023[^\n]", linebuf );
		
		if ( linebuf[0] == 'v' && linebuf[1] == ' ' )
		{
			sscanf( linebuf+2, "%f %f %f", &v_pos[i_pos+0], &v_pos[i_pos+1], &v_pos[i_pos+2] );
			i_pos+=3;
		}
		else if ( linebuf[0] == 'v' && linebuf[1] == 't' )
		{
			sscanf( linebuf+3, "%f %f", &v_uv[i_uv+0], &v_uv[i_uv+1] );
			i_uv+=2;
		}
		else if ( linebuf[0] == 'v' && linebuf[1] == 'n' )
		{
			sscanf( linebuf+3, "%f %f %f", &v_nor[i_nor+0], &v_nor[i_nor+1], &v_nor[i_nor+2] );
			i_nor+=3;
		}
		else if ( linebuf[0] == 'f' )
		{
			if ( n_uv == 0 )
				sscanf( linebuf+2, "%d//%d %d//%d %d//%d", &f_ind[i_f+0], &f_ind[i_f+2], &f_ind[i_f+3], &f_ind[i_f+5], &f_ind[i_f+6], &f_ind[i_f+8] );
			else
				sscanf( linebuf+2, "%d/%d/%d %d/%d/%d %d/%d/%d", &f_ind[i_f+0], &f_ind[i_f+1], &f_ind[i_f+2], &f_ind[i_f+3], &f_ind[i_f+4], &f_ind[i_f+5], &f_ind[i_f+6], &f_ind[i_f+7], &f_ind[i_f+8] );
			for( unsigned int i=0; i< 9; i++ )
				f_ind[i_f+i] -= 1;
			i_f+=9;
		}
		
		bp += strlen( linebuf )+1;
	}
	
	/*Generate mesh*/
	for( unsigned int t=0; t<n_f; t++ )
	{
		for( unsigned int i=0; i<3; i++ )
		{
			m->v[t*3*8 + i*8 + 0] = v_pos[ (f_ind[t*9 +i*3 +0]*3) + 0 ];
			m->v[t*3*8 + i*8 + 1] = v_pos[ (f_ind[t*9 +i*3 +0]*3) + 1 ];
			m->v[t*3*8 + i*8 + 2] = v_pos[ (f_ind[t*9 +i*3 +0]*3) + 2 ];
			
			if ( n_uv == 0 )
			{
				m->v[t*3*8 + i*8 + 6] = 0.0;
				m->v[t*3*8 + i*8 + 7] = 0.0;
			}
			else
			{
				m->v[t*3*8 + i*8 + 6] = v_uv[ (f_ind[t*9 +i*3 +1]*2) + 0 ];
				m->v[t*3*8 + i*8 + 7] = v_uv[ (f_ind[t*9 +i*3 +1]*2) + 1 ];
			}
			
			m->v[t*3*8 + i*8 + 3] = v_nor[ (f_ind[t*9 +i*3 +2]*3) + 0 ];
			m->v[t*3*8 + i*8 + 4] = v_nor[ (f_ind[t*9 +i*3 +2]*3) + 1 ];
			m->v[t*3*8 + i*8 + 5] = v_nor[ (f_ind[t*9 +i*3 +2]*3) + 2 ];
		}
	}
	
	/*Generate bounding box*/
	for( unsigned int i=0; i<n_pos; i+=3 )
	{
		if ( v_pos[i+0] < m->bb[0][0] ) m->bb[0][0] = v_pos[i+0];
		if ( v_pos[i+1] < m->bb[0][1] ) m->bb[0][1] = v_pos[i+1];
		if ( v_pos[i+2] < m->bb[0][2] ) m->bb[0][2] = v_pos[i+2];
		
		if ( v_pos[i+0] > m->bb[1][0] ) m->bb[1][0] = v_pos[i+0];
		if ( v_pos[i+1] > m->bb[1][1] ) m->bb[1][1] = v_pos[i+1];
		if ( v_pos[i+2] > m->bb[1][2] ) m->bb[1][2] = v_pos[i+2];		
	}
	
	m->nv = n_f*3;
	
	free( buf );
	free( v_pos );
	free( v_uv );
	free( v_nor );
	free( f_ind );
	
	printf( "Loaded mesh '%s' with %d vertices\n", filename, m->nv );
	
	return m;
}
