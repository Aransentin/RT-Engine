#include "world.h"
#include "rt_math.h"

#include <stdlib.h>
#include <string.h>

World * world_new( void )
{
	World * w = calloc( 1, sizeof(World) );
	
	return w;
}

Object * object_new( World * w )
{
	w->objects = realloc( w->objects, (w->obj_n+1) * sizeof(Object*) );
	Object * o = calloc( 1, sizeof(Object) );
	w->objects[w->obj_n] = o;
	w->obj_n += 1;
	
	mat4_identity( o->mat );
	return o;
}

void object_attach_mesh( World * w, Object * o, Mesh * m )
{
	o->mesh = m;
	o->dirty = 1;
	w->dirty = 1;
}

unsigned int world_geo_n( World * w )
{
	unsigned int geo_n = 0;
	for( unsigned int i=0; i<w->obj_n; i++ )
	{
		if ( w->objects[i]->mesh )
			geo_n++;
	}
	return geo_n;
}

unsigned int world_vert_n( World * w )
{
	unsigned int v_n = 0;
	for( unsigned int i=0; i<w->obj_n; i++ )
	{
		if ( w->objects[i]->mesh )
		{
			Object * o = w->objects[i];
			o->vo = v_n;
			v_n += o->mesh->nv;
		}
	}
	return v_n;
}

float * world_serialize_bb( World * w )
{
	static float * buf = NULL;
	
	unsigned int geo_n = world_geo_n( w );
	buf = realloc( buf, sizeof(float)*8*geo_n );
	
	unsigned int o = 0;
	for( unsigned int i=0; i<w->obj_n; i++ )
	{
		if ( w->objects[i]->mesh )
		{
			memcpy( buf +o*8, w->objects[i]->bb, sizeof(float)*8 );
			o++;
		}
	}
	
	return buf;
}

int * world_serialize_obj( World * w )
{
	static int * buf = NULL;
	
	unsigned int geo_n = world_geo_n( w );
	buf = realloc( buf, sizeof(int)*2*geo_n );
	
	unsigned int o = 0;
	for( unsigned int i=0; i<w->obj_n; i++ )
	{
		if ( w->objects[i]->mesh )
		{
			buf[o*2+0] = (int) w->objects[i]->vo;
			buf[o*2+1] = (int) w->objects[i]->mesh->nv/3;
			o++;
		}
	}
	
	return buf;
}

float * world_serialize_vert( World * w )
{
	static float * buf = NULL;
	unsigned int vn = world_vert_n( w );
	
	buf = realloc( buf, sizeof(float)*8*vn );
	
	unsigned int v = 0;
	for( unsigned int i=0; i<w->obj_n; i++ )
	{
		if ( w->objects[i]->mesh )
		{
			Object * o = w->objects[i];
			
			memcpy( buf+v*8, o->v, sizeof(float)*8*(o->mesh->nv) );
			v += o->mesh->nv;
		}
	}
	
	return buf;
}

static void object_calculate_bb( Object * o )
{
	o->bb[0] = 32.0f*1024.0f; o->bb[1] = 32.0f*1024.0f; o->bb[2] = 32.0f*1024.0f;
	o->bb[4] = -32.0f*1024.0f; o->bb[5] = -32.0f*1024.0f; o->bb[6] = -32.0f*1024.0f;
	
	for( unsigned int i=0; i<(o->mesh->nv*8); i+=8 )
	{
		if ( o->v[i+0] < o->bb[0] ) o->bb[0] = o->v[i+0];
		if ( o->v[i+1] < o->bb[1] ) o->bb[1] = o->v[i+1];
		if ( o->v[i+2] < o->bb[2] ) o->bb[2] = o->v[i+2];
		
		if ( o->v[i+0] > o->bb[4] ) o->bb[4] = o->v[i+0];
		if ( o->v[i+1] > o->bb[5] ) o->bb[5] = o->v[i+1];
		if ( o->v[i+2] > o->bb[6] ) o->bb[6] = o->v[i+2];
	}
}

void object_vert_transform( Object * o )
{
	for( unsigned int i=0; i<(o->mesh->nv*8); i+=8 )
	{
		mat4_vec3_mul( (o->v+i), o->mat, (o->v+i) );
		mat4_nvec3_mul( (o->v+i+3), o->mat, (o->v+i+3) );
	}
}

void world_recalculate_dirty( World * w )
{
	for( unsigned int i=0; i<w->obj_n; i++ )
	{
		if ( w->objects[i]->dirty )
		{
			Object * o = w->objects[i];
			o->v = realloc( o->v, sizeof(float)*8*o->mesh->nv );
			memcpy( o->v, o->mesh->v, sizeof(float)*8*(o->mesh->nv) );
			
			object_vert_transform( o );
			object_calculate_bb( o );
			
			o->dirty = 0;
		}
	}
}
