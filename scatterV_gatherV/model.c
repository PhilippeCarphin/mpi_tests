#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include "coupler.h"
#include <unistd.h>

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

    /*
     * In GEM and NEMO, each model process only knows its own tile but here
     * since I don't care about tile info exchange, the info of all tiles
     * is hardcoded in the make_tiles function.  Each model process should
     * only use `my_tile`.
     */
    int jpiglo,jpjglo;
    struct tile tiles[4];
    make_tiles(tiles, &jpiglo, &jpjglo);
    struct tile my_tile = tiles[c.local_rank];

    // Wait for coupler to print its stuff
    MPI_Barrier(c.coupler_comm);

    /*
     * Do the receiving MPI_Gatherv
     */
    int recvcount = (my_tile.i1 - my_tile.i0) * (my_tile.j1 - my_tile.j0);
    float *recvbuf = malloc( recvcount * sizeof(*recvbuf));
    fprintf(stderr, "MODEL[%d] (rank:%d), recvcnt=%d, Before MPI_Scattev\n", c.model_no, c.local_rank, recvcount);
    MPI_Scatterv(NULL, NULL, NULL, MPI_FLOAT, recvbuf, recvcount, MPI_FLOAT, 0, c.coupler_comm);

    /*
     * Each model process prints its tile.
     *
     * Everyone does 3 barriers but each model PE prints between a different
     * pair of barriers.
     *
     * Model PE0 prints its tile then does 3 barriers, PE1 does one barrier,
     * prints its tile, then does 2 barriers, and so on.
     */
    switch(c.local_rank){
        case 3: MPI_Barrier(c.local_comm);
        case 2: MPI_Barrier(c.local_comm);
        case 1: MPI_Barrier(c.local_comm);
        case 0: break;
    }
    float *p = recvbuf;
    fprintf(stderr, "Tile of model %d process %d:\n", c.model_no, c.local_rank);
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
    for(int i = 0 ; i < recvcount;i++,p++){
        *p += (1+c.local_rank);
    }

    /*
     * Send modified recvbuf back to the coupler
     */
    MPI_Gatherv(recvbuf, recvcount, MPI_FLOAT, NULL, NULL, NULL, MPI_FLOAT, 0, c.coupler_comm);

    free(recvbuf);

end:
    MPI_Finalize();
    return 0;
}
