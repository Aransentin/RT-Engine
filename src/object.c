#include "object.h"

#include <stdlib.h>

Object * object_new( void )
{
	Object * o = calloc( 1, sizeof(Object) );
	return o;
}
