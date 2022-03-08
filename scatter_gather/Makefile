TRG = scatter_gather

$(TRG): main.c
	mpicc $< -o $@

test: $(TRG)
	mpiexec -n 4 ./$<
