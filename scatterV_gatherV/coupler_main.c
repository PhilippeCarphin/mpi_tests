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
     */
#define index(i,j) (i*jpjglo + j) // row major
    for(int t = 0; t < 4; t++){
        struct tile tt = tiles[t];
        tile_sizes[t] = (tt.i1 - tt.i0)*(tt.j1-tt.j0);
        for(int i = tt.i0; i < tt.i1; i++){
            for(int j = tt.j0; j < tt.j1; j++){
                data[index(i,j)] = tile_value;
            }
        }
        tile_value += 1.111;
    }

    /*
     * Print global data
     */
    for(int i = 0; i < jpiglo; i++){
        for(int j = 0; j < jpjglo; j++){
            fprintf(stderr, "%.2f ", data[index(i,j)]);
        }
        fprintf(stderr, "\n");
    }

    /*
     * Reorder data to make tiles contiguous
     */
    int sendcounts[4];
    int *a = &sendcounts[0];
    int *b = &sendcounts[1];
    fprintf(stderr, "ptr diff b-a = %ld\n", b-a);
    int displs[4];
    float *data_contiguous_tiles = malloc(jpiglo*jpjglo * sizeof(*data_contiguous_tiles));
    float *p = data_contiguous_tiles;
    for(int t = 0; t < 4; t++){
        struct tile tt = tiles[t];
        sendcounts[t] = tile_sizes[t];
        displs[t] = p - data_contiguous_tiles;
        for(int i = tt.i0; i < tt.i1; i++){
            for(int j = tt.j0; j < tt.j1; j++){
                *p++ = data[index(i,j)];
            }
        }

    }
    for(int t = 0; t<4; t++){
        fprintf(stderr, "tile_size[%d]=%d, sendcounts[%d]=%d, displs[%d]=%d\n", t, tile_sizes[t], t, sendcounts[t], t, displs[t]);
    }

    /*
     * Scatter the data
     */
    MPI_Scatterv(data_contiguous_tiles, sendcounts, displs, MPI_FLOAT, NULL, 0, MPI_FLOAT,
            (c.local_rank?MPI_PROC_NULL:MPI_ROOT),
    c.model_comms[0]);

    /*
     * Gather the data
     */
    MPI_Gatherv(NULL, 0, MPI_FLOAT, data_contiguous_tiles, sendcounts, displs, MPI_FLOAT,
            (c.local_rank?MPI_PROC_NULL:MPI_ROOT),
    c.model_comms[0]);

    /*
     * Reorder the data
     */
    p = data_contiguous_tiles;
    for(int t = 0; t < 4; t++){
        struct tile tt = tiles[t];
        for(int i = tt.i0; i < tt.i1; i++){
            for(int j = tt.j0; j < tt.j1; j++){
                data[index(i,j)] = *p++;
            }
        }
    }

    /*
     * Print global data
     */
    for(int i = 0; i < jpiglo; i++){
        for(int j = 0; j < jpjglo; j++){
            fprintf(stderr, "%.2f ", data[index(i,j)]);
        }
        fprintf(stderr, "\n");
    }


    MPI_Finalize();


    return 0;
}
