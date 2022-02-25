#include <mpi.h>
#include <stdio.h>

#define SIZE 100000

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

    MPI_Barrier(MPI_COMM_WORLD);

    fprintf(stderr, "[Rank %d] Hello World\n", world_rank);

    MPI_Barrier(MPI_COMM_WORLD);

    int array[SIZE];
    switch(world_rank){
        case 0:
            MPI_Send(
                    array,
                    SIZE,
                    MPI_INT,
                    1,
                    0,
                    MPI_COMM_WORLD
            );
            break;
        case 1:
            MPI_Recv(array,
                    SIZE,
                    MPI_INT,
                    0,
                    0,
                    MPI_COMM_WORLD,
                    NULL
            );
            fprintf(stderr, "[Rank %d] Got array from 0, sending to 2\n", world_rank);
            MPI_Send(array,
                    SIZE,
                    MPI_INT,
                    2,
                    0,
                    MPI_COMM_WORLD
            );
            break;
        case 2:
            MPI_Recv(array,
                    SIZE,
                    MPI_INT,
                    1,
                    0,
                    MPI_COMM_WORLD,
                    NULL
            );
            fprintf(stderr, "[Rank %d] Got array from 1, sending to 3\n", world_rank);
            MPI_Send(array,
                    SIZE,
                    MPI_INT,
                    3,
                    0,
                    MPI_COMM_WORLD
            );
            break;
        case 3:
            MPI_Recv(array,
                    SIZE,
                    MPI_INT,
                    2,
                    0,
                    MPI_COMM_WORLD,
                    NULL
            );
            fprintf(stderr, "[Rank %d] Got array\n", world_rank);
            break;
    }
    MPI_Finalize();
}



