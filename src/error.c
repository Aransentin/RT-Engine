#include "error.h"

#include <stdlib.h>
#include <stdio.h>
#include "glcorearb.h"
#include <GLFW/glfw3.h>

#define N_ERRORS 47
static char error_strings[N_ERRORS][64] =
{
	"CL_SUCCESS",
	"CL_DEVICE_NOT_FOUND",
	"CL_DEVICE_NOT_AVAILABLE",
	"CL_COMPILER_NOT_AVAILABLE",
	"CL_MEM_OBJECT_ALLOCATION_FAILURE",
	"CL_OUT_OF_RESOURCES",
	"CL_OUT_OF_HOST_MEMORY",
	"CL_PROFILING_INFO_NOT_AVAILABLE",
	"CL_MEM_COPY_OVERLAP",
	"CL_IMAGE_FORMAT_MISMATCH",
	"CL_IMAGE_FORMAT_NOT_SUPPORTED",
	"CL_BUILD_PROGRAM_FAILURE",
	"CL_MAP_FAILURE",
	"CL_INVALID_VALUE",
	"CL_INVALID_DEVICE_TYPE",
	"CL_INVALID_PLATFORM",
	"CL_INVALID_DEVICE",
	"CL_INVALID_CONTEXT",
	"CL_INVALID_QUEUE_PROPERTIES",
	"CL_INVALID_COMMAND_QUEUE",
	"CL_INVALID_HOST_PTR",
	"CL_INVALID_MEM_OBJECT",
	"CL_INVALID_IMAGE_FORMAT_DESCRIPTOR",
	"CL_INVALID_IMAGE_SIZE",
	"CL_INVALID_SAMPLER",
	"CL_INV	ALID_BINARY",
	"CL_INVALID_BUILD_OPTIONS",
	"CL_INVALID_PROGRAM",
	"CL_INVALID_PROGRAM_EXECUTABLE",
	"CL_INVALID_KERNEL_NAME",
	"CL_INVALID_KERNEL_DEFINITION",
	"CL_INVALID_KERNEL",
	"CL_INVALID_ARG_INDEX",
	"CL_INVALID_ARG_VALUE",
	"CL_INVALID_ARG_SIZE",
	"CL_INVALID_KERNEL_ARGS",
	"CL_INVALID_WORK_DIMENSION",
	"CL_INVALID_WORK_GROUP_SIZE",
	"CL_INVALID_WORK_ITEM_SIZE",
	"CL_INVALID_GLOBAL_OFFSET",
	"CL_INVALID_EVENT_WAIT_LIST",
	"CL_INVALID_EVENT",
	"CL_INVALID_OPERATION",
	"CL_INVALID_GL_OBJECT",
	"CL_INVALID_BUFFER_SIZE",
	"CL_INVALID_MIP_LEVEL",
	"CL_INVALID_GLOBAL_WORK_SIZE"
};

static cl_int error_nums[N_ERRORS] =
{
	0,
	-1,
	-2,
	-3,
	-4,
	-5,
	-6,
	-7,
	-8,
	-9,
	-10,
	-11,
	-12,
	-30,
	-31,
	-32,
	-33,
	-34,
	-35,
	-36,
	-37,
	-38,
	-39,
	-40,
	-41,
	-42,
	-43,
	-44,
	-45,
	-46,
	-47,
	-48,
	-49,
	-50,
	-51,
	-52,
	-53,
	-54,
	-55,
	-56,
	-57,
	-58,
	-59,
	-60,
	-61,
	-62,
	-63
};

void error_die( const char * message )
{
	printf( "Error: %s\n", message );
	exit( 1 );
}

void error_GL( void )
{
	GLenum error = glGetError();
	if ( error != GL_NO_ERROR )
		printf( "Error: OpenGL error %i\n", error );
}

void error_cl( int line, cl_int err )
{
	if ( err != CL_SUCCESS )
	{
		const char * errname = "Undefined";
		for( int i=0; i<N_ERRORS; i++ )
		{
			if ( err == error_nums[i] )
			{
				errname = error_strings[i];
				break;
			}
		}
		printf( "Error on line %i: %i (%s)\n", line, err, errname );
		exit( 1 );
	}
}
