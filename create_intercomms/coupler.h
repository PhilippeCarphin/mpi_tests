#include <mpi.h>
enum Role { R_MODEL, R_COUPLER};

struct coupler {
    enum Role role;
    MPI_Comm local_comm;
    int local_rank;
    int local_size;

    // Coupler
    int nb_models;
    MPI_Comm model_comms[8];
    int model_sizes[8];

    // Model
    int model_no;
    int rank;
    MPI_Comm coupler_comm;
};

struct message {
    int i0,j0;
    int i1,j1;
};

struct tile {
    int i0,j0;
    int i1,j1;
};

MPI_Comm init_coupler(struct coupler *c, int nb_models);
void make_tiles(struct tile *tiles, int *jpiglo, int *jpjglo);
