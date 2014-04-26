#pragma once
#include <CL/cl.h>
#include <CL/cl_gl.h>

void error_die( const char * message ) __attribute__((noreturn));
void error_GL( void );
void error_cl( int line, cl_int err );
