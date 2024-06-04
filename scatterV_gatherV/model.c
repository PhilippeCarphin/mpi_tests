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

    /*
     * The other models are only there to test the creation of inter-communicators
     */
    if(c.model_no != 0){
        goto end;
    }

    int jpiglo,jpjglo;
    struct tile tiles[4];
    make_tiles(tiles, &jpiglo, &jpjglo);

    struct tile my_tile = tiles[c.local_rank];
    int recvcount = (my_tile.i1 - my_tile.i0) * (my_tile.j1 - my_tile.j0);
    float *recvbuf = malloc( recvcount * sizeof(*recvbuf));
    fprintf(stderr, "Model %d (rank:%d), recvcnt=%d, Before MPI_Scattev\n", c.model_no, c.local_rank, recvcount);
    MPI_Scatterv(NULL, NULL, NULL, MPI_FLOAT, recvbuf, recvcount, MPI_FLOAT, 0, c.coupler_comm);

    /*
     * Print data.  Each PROC does 4 MPI_Barrier() calls with printing
     * done between different barriers each
     * proc 0 prints
     * everyone barrier
     * proc 1 prints
     * everyone barrier
     * proc 2 prints
     * everyone barrier
     * proc 3 prints
     * etc.
     */
    switch(c.local_rank){
        case 3: MPI_Barrier(c.local_comm);
        case 2: MPI_Barrier(c.local_comm);
        case 1: MPI_Barrier(c.local_comm);
        case 0: break;
    }
    float *p = recvbuf;
    for(int i = my_tile.i0; i <my_tile.i1; i++){
        for(int j = my_tile.j0; j < my_tile.j1; j++){
            fprintf(stderr, "%.2f ", *p++);
        }
        fprintf(stderr, "\n");
    }
    switch(c.local_rank){
        case 0: MPI_Barrier(c.local_comm);
        case 1: MPI_Barrier(c.local_comm);
        case 2: MPI_Barrier(c.local_comm);
        case 3: break;
    }

    /*
     * Double every entry in recvbuf
     */
    p = recvbuf;
    for(int i = my_tile.i0; i <my_tile.i1; i++){
        for(int j = my_tile.j0; j < my_tile.j1; j++){
            *p++ = 1 + c.local_rank + (2 * c.local_rank/10.0);
        }
    }

    /*
     * Send doubled recvbuf back to the coupler
     */
    MPI_Gatherv(recvbuf, recvcount, MPI_FLOAT, NULL, NULL, NULL, MPI_FLOAT, 0, c.coupler_comm);

end:
    MPI_Finalize();
    return 0;
}
