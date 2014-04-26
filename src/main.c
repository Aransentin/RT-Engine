#include "engine.h"
#include "world.h"
#include "mesh.h"
#include "rt_math.h"

int main()
{
	Engine * e = engine_new( 512, 512, 512/4, 512/4 );
	World * w = world_new();
	e->world = w;
	
	Mesh * m1 = mesh_load_obj( "assets/testarossa.obj" );
	Mesh * m2 = mesh_load_obj( "assets/ferarri.obj" );
	Mesh * m3 = mesh_load_obj( "assets/popo.obj" );
	Mesh * m4 = mesh_load_obj( "assets/plane.obj" );
	
	Object * po = object_new( w );
	object_attach_mesh( w, po, m4 );
	
	for( int i=0; i<3; i++ )
	{
		Object * o = object_new( w );
		mat4_translate( o->mat, 0.0f, i*4.0f - 4.0f, 0.0f );

		if ( i == 0 )
		{
			mat4_rotate_Z( o->mat, o->mat, 0.3f );
			object_attach_mesh( w, o, m1 );
		}
		else if ( i == 1 )
		{
			object_attach_mesh( w, o, m3 );
		}
		else
		{
			mat4_rotate_Z( o->mat, o->mat, -0.3f );
			object_attach_mesh( w, o, m2 );
		}
	}
	
	while( 1 )
		engine_render( e );
}
