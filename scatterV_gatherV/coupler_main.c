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

    struct tile tiles[4];
    int jpiglo;
    int jpjglo;
    make_tiles(tiles, &jpiglo, &jpjglo);

    float *data = malloc(jpiglo*jpjglo * sizeof(*data));
    float tile_value = 1.111;
    int tile_sizes[4];
    /*
     * Fill global data with same value across single tiles
     * tile[0] gets 1.11 everywhere, tile[1] gets 2.22 and so on.
     */
#define ROW_MAJOR
#ifdef ROW_MAJOR
#define index(i,j) (i*jpjglo + j) // row major with i varying vertically
                                  // and j varying horizontally
#else
#define index(i,j) (j*jpiglo + i)
#endif
    for(int t = 0; t < 4; t++){
        struct tile tt = tiles[t];
        tile_sizes[t] = (tt.i1 - tt.i0)*(tt.j1-tt.j0);
        // Set all values in the tile to tile_value
        for(int i = tt.i0; i < tt.i1; i++){
            for(int j = tt.j0; j < tt.j1; j++){
                data[index(i,j)] = tile_value;
            }
        }
        // Increase tile_value for the next tile
        tile_value += 1.111;
    }

    /*
     * Print global data
     */
    fprintf(stderr, "Coupler: global grid being distributed to tiles of Model 0\n");
    for(int i = 0; i < jpiglo; i++){
        for(int j = 0; j < jpjglo; j++){
            fprintf(stderr, "%.2f ", data[index(i,j)]);
        }
        fprintf(stderr, "\n");
    }

    /*
     * Reorder data to make tiles contiguous.  While doing this, we save the
     * values of the arguments sendcnts and displs
     */
    float *data_contiguous_tiles = malloc(jpiglo*jpjglo * sizeof(*data_contiguous_tiles));
    int sendcnts[4];
    int displs[4];
    float *p = data_contiguous_tiles;
    for(int t = 0; t < 4; t++){
        struct tile tt = tiles[t];
        sendcnts[t] = tile_sizes[t];
        displs[t] = p - data_contiguous_tiles;
        // Copy one tile as contiguously into a section of data_contiguous_tiles
        for(int i = tt.i0; i < tt.i1; i++){
            for(int j = tt.j0; j < tt.j1; j++){
                *p++ = data[index(i,j)];
            }
        }

    }

    for(int t = 0; t<4; t++){
        fprintf(stderr, "COUPLER: tile_size[%d]=%d, sendcnts[%d]=%d, displs[%d]=%d\n", t, tile_sizes[t], t, sendcnts[t], t, displs[t]);
    }
    /*
     * Synchronize prints: Coupler prints its sendcounts and displs before this
     * barrier.
     */
    MPI_Barrier(c.model_comms[0]);
    /*
     * Model 0 processes print their recvcounts and displs
     */

    /*
     * Scatter the data to model 1
     * Coupler process 0 get MPI_ROOT and all the other ones get MPI_PROC_NULL
     */
    MPI_Scatterv(
        data_contiguous_tiles, sendcnts, displs, MPI_FLOAT,
        NULL, 0, MPI_FLOAT,
        (c.local_rank == 0 ? MPI_ROOT : MPI_PROC_NULL),
        c.model_comms[0]
    );

    /*
     * Model 0 processes each print their tile
     */

    /*
     * Model 0 processes make a random change to their tile.
     * In this case, models add local_rank+1 to each value in their tile
     * so we can detect if each model is working on the right data
     */

    /*
     * Gather the data back from model 0
     */
    const int *recvcnts = sendcnts;
    MPI_Gatherv(
        NULL, 0, MPI_FLOAT,
        data_contiguous_tiles, sendcnts, displs, MPI_FLOAT,
        (c.local_rank == 0 ? MPI_ROOT : MPI_PROC_NULL),
        c.model_comms[0]
    );

    /*
     * Reorder the data
     */
    p = data_contiguous_tiles;
    for(int ti = 0; ti < 4; ti++){
        struct tile t = tiles[ti];
        for(int i = t.i0; i < t.i1; i++){
            for(int j = t.j0; j < t.j1; j++){
                data[index(i,j)] = *p++;
            }
        }
    }

    /*
     * Print global data
     */
    fprintf(stderr, "Coupler: Printing global grid gathered from tiles of model 0\n");
    for(int i = 0; i < jpiglo; i++){
        for(int j = 0; j < jpjglo; j++){
            fprintf(stderr, "%.2f ", data[index(i,j)]);
        }
        fprintf(stderr, "\n");
    }

    MPI_Finalize();

    free(data_contiguous_tiles);
    free(data);

    return 0;
}
