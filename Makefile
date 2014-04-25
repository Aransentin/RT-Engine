all:
	clang -g --std=c11 -Weverything src/*.c -lOpenCL -lglfw -lm -lGL -lpng
