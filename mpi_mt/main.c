#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    int provided, rank;
    MPI_Comm comm = MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);


    if(rank == 0){
        fprintf(stderr, "Value of provided ");
        switch(provided){
            case MPI_THREAD_SINGLE: fprintf(stderr, "MPI_THREAD_SINGLE"); break;
            case MPI_THREAD_FUNNELED: fprintf(stderr, "MPI_THREAD_FUNNELED"); break;
            case MPI_THREAD_SERIALIZED: fprintf(stderr, "MPI_THREAD_SERIALIZED"); break;
            case MPI_THREAD_MULTIPLE: fprintf(stderr, "MPI_THREAD_MULTIPLE"); break;
        }
        fprintf(stderr, "\n");
        if(provided == MPI_THREAD_MULTIPLE){
            fprintf(stderr, "Youppi douppi notre MPI est capable de supporter le multi-thread si on lui demande\n");
        }
    }

    return 0;
}
