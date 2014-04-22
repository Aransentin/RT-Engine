##RT-Engine

A realtime C/OpenCL-OpenGL interop raytracer.

Requires Linux (for now), in addition to the GLFW and OpenCL libraries. So far, it has been tested on the nVidia & Intel OpenCL platforms.

A GPU that supports 'cl_khr_gl_sharing' is recommended, as it will otherwise fallback to manual texture transfer - this will be quite slow.

#####How to compile:

gcc --std=c11 *.c -lOpenCL -lglfw -lm -lGL

#####Sample screenshot
![RT-Engine]( http://i.imgur.com/UobOFxy.png )