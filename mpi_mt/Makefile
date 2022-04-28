all: main

main: main.c
	mpicc $^ -o $@

test: main
	./$<
