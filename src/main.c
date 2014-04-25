#include "engine_core.h"

int main()
{
	/*Start engine.*/
	size_t dim[4] = { 512, 512, 512, 512 };
	Engine * e = engine_init( dim );
	
	/*Create world, bind to engine.*/
	World * w = world_new();
	e->world = w;
	
	/*Make meshes and place them in the world*/
	Object * o1 = object_new();
	Object * o2 = object_new();
	Object * o3 = object_new();
	
	Mesh * m1 = mesh_load_obj( "assets/ferarri.obj" );
	Mesh * m2 = mesh_load_obj( "assets/testarossa.obj" );
	Mesh * m3 = mesh_load_obj( "assets/plane.obj" );
	
	o1->mesh = m1;
	o2->mesh = m2;
	o3->mesh = m3;
	
	world_addObject( w, o1 );
	world_addObject( w, o2 );
	world_addObject( w, o3 );
	
	while( 1 )
		engine_render( e );
}
