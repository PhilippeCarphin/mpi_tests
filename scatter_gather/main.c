#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

const int part_size = 3;

typedef struct MPI_Helper {
    int mpi_size;
    int mpi_rank;

    int data_size;
    int *data_in;        // Keeping them separate for experimentation
    int *data_out;

    int data_part_size;
    int *data_part_in;
    int *data_part_out;

} MPI_Helper_t;

int mpi_helper_init(MPI_Helper_t *mpi_helper)
{
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_helper->mpi_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_helper->mpi_rank);

    mpi_helper->data_size = part_size * mpi_helper->mpi_size;
    mpi_helper->data_part_size = part_size;

    if(mpi_helper->mpi_rank == 0){
        mpi_helper->data_in  = malloc(mpi_helper->data_size * sizeof(*(mpi_helper->data_in)));
        mpi_helper->data_out = malloc(mpi_helper->data_size * sizeof(*(mpi_helper->data_out)));
    } else {
        mpi_helper->data_in = NULL;
        mpi_helper->data_out = NULL;
    }

    mpi_helper->data_part_in = malloc(mpi_helper->data_part_size * sizeof(*(mpi_helper->data_in)));
    mpi_helper->data_part_out = malloc(mpi_helper->data_part_size * sizeof(*(mpi_helper->data_out)));

    if(mpi_helper->mpi_rank == 0){
        fprintf(stderr, "world_size = %d\n", mpi_helper->mpi_size);
    }
    fprintf(stderr, "world_rank = %d\n", mpi_helper->mpi_rank);

    return 0;
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    MPI_Helper_t mpi_helper;
    mpi_helper_init(&mpi_helper);

    // Root process initializes data
    if(mpi_helper.mpi_rank == 0){
        for(int i = 0; i < mpi_helper.data_size; i++){
            mpi_helper.data_in[i] = (i * mpi_helper.mpi_size)/mpi_helper.data_size;
        }

        for(int i = 0; i < mpi_helper.data_size; i++){
            fprintf(stderr, "mpi_helper.data_in[%d] = %d\n", i, mpi_helper.data_in[i]);
        }
    }

    MPI_Scatter(
        mpi_helper.data_in,      mpi_helper.data_part_size, MPI_INT,
        mpi_helper.data_part_in, mpi_helper.data_part_size, MPI_INT,
        0, // root
        MPI_COMM_WORLD
    );

    for(int i = 0; i < part_size; i++){
        // if(mpi_helper.data_in[i] != mpi_helper.mpi_rank){
        //     fprintf(stderr, "[Rank %d] Unexpected data_in[%d] : %d\n", mpi_helper.mpi_rank, i, mpi_helper.data_in[i]);
        // }
        mpi_helper.data_part_out[i] = mpi_helper.data_part_in[i] * mpi_helper.mpi_rank;
    }

    MPI_Gather(
        mpi_helper.data_part_out, mpi_helper.data_part_size, MPI_INT,
        mpi_helper.data_out,      mpi_helper.data_part_size, MPI_INT,
        0, // root
        MPI_COMM_WORLD
    );

    if(mpi_helper.mpi_rank == 0){
        for(int i = 0; i < mpi_helper.data_size; i++){
            fprintf(stderr, "mpi_helper.data_out[%d] = %d\n", i, mpi_helper.data_out[i]);
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Finalize();

    return 0;
}
