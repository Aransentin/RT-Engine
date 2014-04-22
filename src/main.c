#include "engine_core.h"

int main()
{
	/*Start engine.*/
	size_t dim[4] = { 512, 512, 512, 512 };
	Engine * e = engine_init( dim );
	
	/*Create world, bind to engine.*/
	World * w = world_new();
	e->world = w;
	
	/*Make a mesh, and place it in the world*/
	Object * o = object_new();
	Mesh * m = mesh_load_obj( "assets/agdg.obj" );
	o->mesh = m;
	world_addObject( w, o );
	
	while( 1 )
		engine_render( e );
}
