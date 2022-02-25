
hello: main.c
	mpicc $< -o $@

test: hello
	mpiexec -n 4 ./$<
