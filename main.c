#include "engine_core.h"

int main()
{
	size_t dim[4] = { 512, 512, 512, 512 };
	Engine * e = engine_init( dim );
	
	while( 1 )
		engine_render( e );
}
