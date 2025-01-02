#include <mpi.h>
#include "coupler.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

MPI_Comm init_coupler(struct coupler *c, int nb_models)
{
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int *v;
    int appnum, flag=0;
    MPI_Comm_get_attr(MPI_COMM_WORLD, MPI_APPNUM, &v, &flag);
    appnum = *v;

    /*
     * Create local_comm, communicator between processes with the same value
     * of `appnum`.  These are the groups of processes launched in the mpirun
     * command.
     */
    MPI_Comm_split(MPI_COMM_WORLD, appnum, world_rank, &c->local_comm);
    MPI_Comm_rank(c->local_comm, &c->local_rank);
    MPI_Comm_size(c->local_comm, &c->local_size);
    usleep(10000 * world_rank); // sleep a an amount of time proportional to world_rank so processes print themselves in order
    fprintf(stderr, "world_rank=%2.d, appnum=%d, local_rank=%d\n", world_rank, appnum, c->local_rank);

    c->role = (appnum == 0 ? R_COUPLER : R_MODEL);

    /*
     * Each process shares the size of its group to the coupler.
     */
    int *all_sizes;
    if(c->role == R_COUPLER){
        all_sizes = malloc(world_size * sizeof(*all_sizes));
    }
    MPI_Gather(&c->local_size, 1, MPI_INT, all_sizes, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if(c->role == R_COUPLER){
        /*
         * Print all sizes array
         */
        fprintf(stderr, "All sizes   = [");
        for(int i = 0; i<world_size; i++){
            fprintf(stderr, "%2.d ", all_sizes[i]);
        }
        fprintf(stderr, "\033[1D]\n");
        /*
         * Print ranks below sizes array to confirm values are correct
         */
        fprintf(stderr, "World ranks = [");
        fprintf(stderr, " 0 "); // "%2.d" prints just a space if the int is 0
        for(int i = 1; i<world_size; i++){
            fprintf(stderr, "%2.d ", i);
        }
        fprintf(stderr, "\033[1D]\n");

        /*
         * The coupler ends up with an array like this, and we want to use this
         * to make the array [n1, n2, n3, ...].
         * ==> all_sizes = [nc ... nc n1 n1 n1 ... n1 n2 n2 n2 ... n2 n3 n3 n3 ... n3 ...]
         *                  ^         |--n1 entries-| |--n2 entries-| |--n3 entries-| ...
         *                  |         ^               ^               ^
         *       index:     0        0+nc             i              i+n2
         *      memory:(all_sizes) (all_sizes+nc)     p              p+*p
         * where
         *  - nc is the number of coupler processes
         *  - n1,n2... are the number of processes of each model
         */
        int model_sizes[nb_models];
        int coupler_size = c->local_size;
        int *p = all_sizes + coupler_size;
        for(int i = 0; i < nb_models ; i++){
            c->model_sizes[i] = *p;
            /* If the current model has size `*p`, then it will have `*p` entries
             * in the all_sizes array.  Therefore we need to advance by `*p`
             * elements to reach the first entry of the next model */
            p += *p;
        }

        /*
         * Print array of model sizes
         */
        fprintf(stderr, "Model sizes = [");
        for(int i = 0; i<nb_models; i++){
            fprintf(stderr, "%d ", c->model_sizes[i]);
        }
        fprintf(stderr, "\033[1D]\n");
    }

    /*
     * Create intercommunicators.  If I'm a model, I make one call to
     * MPI_Intercomm_create and the coupler will make one call to
     * per model
     */
    if(c->role == R_MODEL){
        // appnum = 1,2,... (because 0 is coupler and every one after is a model)
        MPI_Intercomm_create(
                c->local_comm,    // local_comm,
                0,                // local leader
                MPI_COMM_WORLD,   // pier comm
                0,                // remote leader,
                appnum,           // Tag
                &c->coupler_comm  // New intercomm
        );
        c->model_no = appnum-1; // Make it start at 0 so it can be used as an index;
    } else if (c->role == R_COUPLER){
        int remote_leader_world_rank = all_sizes[0]; // 
        for(int i = 0; i < nb_models; i++){
            fprintf(stderr, "Model %d's remote_leader_world_rank is %d\n", i, remote_leader_world_rank);
            MPI_Intercomm_create(
                    c->local_comm,             // local_comm
                    0,                         // local_leader
                    MPI_COMM_WORLD,            // pier_comm
                    remote_leader_world_rank,  // remote_leader
                    i+1,                       // Tag: 1,2,... corresponding to the appnums of each model
                    &c->model_comms[i]         // New intercomm
            );
            // int this_model_size = all_sizes[remote_leader_world_rank];
            // remote_leader_world_rank += this_model_size;
            remote_leader_world_rank += c->model_sizes[i];
        }
    }

    if(c->role == R_COUPLER){
        free(all_sizes);
    }

    return c->local_comm;
}

/*
 * Lazy way of sharing grid and tile information between the coupler and the
 * model.  This thing is just to learn to use MPI_Gatherv, MPI_Scatterv so I can
 * start using it in the real coupler.
 */
void make_tiles(struct tile *tiles, int *jpiglo, int *jpjglo){
    *jpiglo = 11;
    *jpjglo = 13;
    int jpni = 2;
    int jpnj = 2;
    tiles[0] = (struct tile){
        .i0 = 0,
        .j0 = 0,
        .i1 = *jpiglo/2,
        .j1 = *jpjglo/2
    };
    tiles[1] = (struct tile){
        .i0 = *jpiglo/2,
        .j0 = 0,
        .i1 = *jpiglo,
        .j1 = *jpjglo/2
    };
    tiles[2] = (struct tile){
        .i0 = 0,
        .j0 = *jpjglo/2,

        .i1 = *jpiglo/2,
        .j1 = *jpjglo
    };
    tiles[3] = (struct tile){
        .i0 = *jpiglo/2,
        .j0 = *jpjglo/2,

        .i1 = *jpiglo,
        .j1 = *jpjglo
    };
}
