#include <mpi.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    int init_result = MPI_Init(&argc, &argv);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    if(world_rank == 0){
        fprintf(stderr, "[Rank %d] size is %d\n", world_rank, world_size);
    }

    fprintf(stderr, "[Rank %d] Hello World\n", world_rank);

    MPI_Finalize();
}



