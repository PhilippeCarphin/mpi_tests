#include <mpi.h>
#include "coupler.h"
#include <stdlib.h>
#include <stdio.h>

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
    fprintf(stderr, "appnum = %d\n", appnum);

    MPI_Comm_split(MPI_COMM_WORLD, appnum, world_rank, &c->local_comm);

    MPI_Comm_rank(c->local_comm, &c->local_rank);
    MPI_Comm_size(c->local_comm, &c->local_size);
    fprintf(stderr, "world_rank=%d, local_rank=%d\n", world_rank, c->local_rank);

    c->role = (appnum == 0 ? R_COUPLER : R_MODEL);

    int *all_sizes;
    if(c->role == R_COUPLER){
        all_sizes = malloc(world_size * sizeof(*all_sizes));
    }
    MPI_Gather(&c->local_size, 1, MPI_INT, all_sizes, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if(c->role == R_COUPLER){
        fprintf(stderr, "All sizes = [");
        for(int i = 0; i<world_size; i++){
            fprintf(stderr, "%d ", all_sizes[i]);
        }
        fprintf(stderr, "\033[1D]\n");
        int model_sizes[nb_models];
        int coupler_size = c->local_size;
        int *p = all_sizes + coupler_size;
        for(int i = 0; i < nb_models ; i++){
            model_sizes[i] = *p;
            p += *p;
        }
        fprintf(stderr, "Model sizes = [");
        for(int i = 0; i<nb_models; i++){
            fprintf(stderr, "%d ", model_sizes[i]);
        }
        fprintf(stderr, "\033[1D]\n");
    }

    if(c->role == R_MODEL){
        // appnum = 1,2,... (because 0 is coupler and every one after is a model)
        MPI_Intercomm_create(
                c->local_comm, // local_comm,
                0, // local leader
                MPI_COMM_WORLD,
                0, // remote leader,
                appnum, // Tag
                &c->coupler_comm
        );
        c->model_no = appnum-1; // Make it start at 0 so it can be used as an index;
    } else if (c->role == R_COUPLER){
        int remote_leader_world_rank = all_sizes[0];
        for(int i = 0; i < nb_models; i++){
            fprintf(stderr, "Model %d's remote_leader_world_rank is %d\n", i, remote_leader_world_rank);
            int this_model_size = all_sizes[remote_leader_world_rank];
            MPI_Intercomm_create(
                    c->local_comm,
                    0,
                    MPI_COMM_WORLD,
                    remote_leader_world_rank,
                    i+1, // 1,2,... corresponding to the appnums of each model
                    &c->model_comms[i]
            );
            remote_leader_world_rank += this_model_size;
        }
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
