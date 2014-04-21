#include "engine_core.h"

int main()
{
	size_t dim[4] = { 512, 512, 512, 512 };
	Engine * e = engine_init( dim );
	
	World * w = world_new();
	e->world = w;
	
	Object * o = object_new();
	Mesh * m = mesh_load_obj( "assets/agdg.obj" );
	o->mesh = m;
	world_addObject( w, o );
	
	while( 1 )
		engine_render( e );
}
