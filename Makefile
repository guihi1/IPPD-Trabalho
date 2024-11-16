CC=gcc

FLAGS=-O3 -lm -fopenmp

all: histogram

histogram: histogram.c
	$(CC) $(FLAGS) histogram.c -o histogram

clean:
	rm histogram
