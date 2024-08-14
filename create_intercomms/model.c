#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include "coupler.h"

int main(int argc, char **argv)
{
    int err = MPI_Init(&argc, &argv);
    if(err){
        fprintf(stderr, "Failed to intialize MPI\n");
    }

    struct coupler c;
    init_coupler(&c, 0);

    MPI_Finalize();

    return 0;
}
