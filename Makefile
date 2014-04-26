linux:
	clang -g --std=c11 -Weverything -Wno-padded src/*.c -lOpenCL -lglfw -lm -lGL -lpng	
