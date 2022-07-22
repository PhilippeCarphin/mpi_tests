#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

const int TRUE = 1;
const int FALSE = 0;

const int part_size = 4;

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

/*
 * Set the data so that the values are equal to the rank of the MPI
 * process that will get that chunk of data without having each process
 * set that value
 */
int mpi_helper_init_data_in(MPI_Helper_t *h){

    if(h->mpi_rank == 0){
        for(int chunk_id = 0; chunk_id < h->mpi_size; chunk_id++){
            int chunk_start = chunk_id * h->data_part_size;
            int chunk_end = (chunk_id+1) * h->data_part_size;
            for(int j = chunk_start ; j < chunk_end ; j++){
                h->data_in[j] = chunk_id;
            }
        }
    }
    return 0;
}

/*
 * Process 0 prints data in
 */
int mpi_helper_print_and_check_data_in(MPI_Helper_t *h){
    if(h->mpi_rank == 0){
        for(int chunk_id = 0; chunk_id < h->mpi_size; chunk_id++){
            int chunk_start = chunk_id * h->data_part_size;
            int chunk_end = (chunk_id+1) * h->data_part_size;
            for(int j = chunk_start; j < chunk_end; j++){
                int expected = chunk_id;
                if(h->data_in[j] != expected) {
                    fprintf(stderr, "h->data_in[%d] = %d different from expected %d\n", j, h->data_in[j], expected);
                }
                fprintf(stderr, "h->data_in[%d] = %d\n", j, h->data_in[j]);
            }
        }
    }
    return 0;
}

/*
 * Process 0 prints data in
 */
int mpi_helper_print_and_check_data_out(MPI_Helper_t *h){
    if(h->mpi_rank == 0){
        for(int chunk_id = 0; chunk_id < h->mpi_size; chunk_id++){
            int chunk_start = chunk_id * h->data_part_size;
            int chunk_end = (chunk_id+1) * h->data_part_size;
            for(int j = chunk_start; j < chunk_end; j++){
                int expected = chunk_id * chunk_id;
                if(h->data_out[j] != expected) {
                    fprintf(stderr, "h->data_out[%d] = %d different from expected %d\n", j, h->data_out[j], expected);
                } else {
                    fprintf(stderr, "h->data_out[%d] = %d\n", j, h->data_out[j]);
                }
            }
        }
    }
    return 0;
}

/*
 * Represents the work that a single MPI process would do for its data
 */
int mpi_helper_process_data_part(MPI_Helper_t *h, int add_error){
    for(int i = 0; i < part_size; i++){
        if(h->data_part_in[i] != h->mpi_rank){
            fprintf(stderr, "[Rank %d] Unexpected data_in[%d] (values should be equal to rank): %d\n", h->mpi_rank, i, h->data_part_in[i]);
        }
    }

    // The process does something to its chunk of data
    for(int i = 0; i < part_size; i++){
        int error = 0;
        if(add_error){
            unsigned int t = (unsigned int)time(NULL);
            int r = rand_r(&t + h->mpi_rank);
            error = (r > 1500000000);
        }
        h->data_part_out[i] = h->data_part_in[i] * h->mpi_rank + error;
        // usleep(h->data_part_out[i] * 1000);
    }
    return 0;
}

/*
 * Stores rank and size info in the corresponding fields and allocates
 * memory for the various arrays.
 */
int mpi_helper_init(MPI_Helper_t *h)
{
    MPI_Comm_size(MPI_COMM_WORLD, &h->mpi_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &h->mpi_rank);

    h->data_size = part_size * h->mpi_size;
    h->data_part_size = part_size;

    if(h->mpi_rank == 0){
        h->data_in  = malloc(h->data_size * sizeof(*(h->data_in)));
        h->data_out = malloc(h->data_size * sizeof(*(h->data_out)));
    } else {
        h->data_in = NULL;
        h->data_out = NULL;
    }

    h->data_part_in = malloc(h->data_part_size * sizeof(*(h->data_in)));
    h->data_part_out = malloc(h->data_part_size * sizeof(*(h->data_out)));

    if(h->mpi_rank == 0){
        fprintf(stderr, "world_size = %d\n", h->mpi_size);
        fprintf(stderr, "sizeof(int) = %ld\n", sizeof(int));
    }
    fprintf(stderr, "world_rank = %d\n", h->mpi_rank);

    return 0;
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    MPI_Helper_t h;
    mpi_helper_init(&h);

    // Root process initializes data
    mpi_helper_init_data_in(&h);

    mpi_helper_print_and_check_data_in(&h);

    // Each process gets a different chunk of data_in sent by the root process
    // to its data_part_in.  Note that the root process also sends itself a
    // chunk of data.
    if(h.mpi_rank == 0){
        MPI_Scatter(
            h.data_in,      h.data_part_size, MPI_INT,
            h.data_part_in, h.data_part_size, MPI_INT,
            0, // root
            MPI_COMM_WORLD
        );
    } else {
        MPI_Scatter(
            h.data_in,      h.data_part_size, MPI_INT,
            h.data_part_in, h.data_part_size, MPI_INT,
            0, // root
            MPI_COMM_WORLD
        );
    }
    // NOTE: The total data size is implicit from the part size and the number
    // of involved processes
    // for(int dest_rank = 0; dest_rank < h.mpi_size; dest_rank++){
    //      int send_start = &(h.data_in[dest_rank * h.data_part_size]);
    //      MPI_Send(
    //          send_start, h.data_part_size, MPI_INT,
    //          dest_rank, // destination
    //          0, // Tag (irrelevant)
    //          MPI_WORLD_COMM
    //      )
    //  }

    // Now each process has a data_part_in whose values should all be equal
    // to its rank
    int add_error = TRUE;
    mpi_helper_process_data_part(&h, add_error);

    // Each process will send its data_part_out to data_out on the root process
    // Note that the root process sends its data_part_out to itself.
    if(h.mpi_rank == 0){
        MPI_Gather(
            h.data_part_out, h.data_part_size, MPI_INT,
            h.data_out,      h.data_part_size, MPI_INT,
            0, // root
            MPI_COMM_WORLD
        );
    } else {
        MPI_Gather(
            h.data_part_out, h.data_part_size, MPI_INT,
            h.data_out,      h.data_part_size, MPI_INT,
            0, // root
            MPI_COMM_WORLD
        );
    }

    // The data should be reassembled in order it was sent.
    mpi_helper_print_and_check_data_out(&h);

    usleep(10);
    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Finalize();

    return 0;
}
