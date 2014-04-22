all:
	gcc --std=c11 src/*.c -lOpenCL -lglfw -lm -lGL
