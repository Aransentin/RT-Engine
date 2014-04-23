all:
	gcc --std=gnu11 src/*.c -lOpenCL -lglfw -lm -lGL
