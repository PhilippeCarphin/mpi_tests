#include <mpi.h>
#include "coupler.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{

    if(argc < 2){
        fprintf(stderr, "nb_model required\n");
        return 1;
    }

    int nb_models;
    if(sscanf(argv[1], "%d", &nb_models) != 1){
        fprintf(stderr, "First arg must be an integer for the number of models\n");
        return 1;
    }

    int err = MPI_Init(&argc, &argv);
    if(err){
        fprintf(stderr, "Failed to intialize MPI\n");
    }

    struct coupler c;
    init_coupler(&c, nb_models);

    MPI_Finalize();

    return 0;
}
