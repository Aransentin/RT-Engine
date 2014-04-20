##RT-Engine

A realtime C/OpenCL-OpenGL interop raytracer.

Requires Linux (for now), a GPU that supports cl_khr_gl_sharing, in addition to the GLFW and OpenCL libraries.

#####How to compile:

gcc --std=c11 *.c -lOpenCL -lglfw -lm -lGL

#####Sample screenshot
![RT-Engine]( http://i.imgur.com/0haa75V.png )