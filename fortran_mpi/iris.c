#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include "iris.h"

// https://github.com/LadaF/PoisFFT/blob/master/src/f_mpi_comm_c2f.c

MPI_Fint Fortran_Iris_Model_Init(TIris *Iris, int ModelNo) {
    Iris->ModelNo = ModelNo;
    fprintf(stderr, "%s() c:MPI_COMM_WORLD %d\n", __func__, MPI_COMM_WORLD);
    // LadaF notes that MPI_Comm_c2f could be a macro
    // MPI_COMM_WORLD is used as an example
    MPI_Fint retval = MPI_Comm_c2f(MPI_COMM_WORLD);
    fprintf(stderr, "%s() returning %d\n", __func__, retval);
    return retval;
}

MPI_Comm Iris_Model_Init(TIris *Iris, int ModelNo){
    // Represents something that does mpi_init, splits communicators and stuff
    // and stores two communicators in a struct.
    //
    // Here just do mpi_init and return the communicator
    //
    Iris->ModelNo = ModelNo;
    return MPI_COMM_WORLD;
}

int Fortran_Iris_Get_ModelNo(TIris *Iris) {
    return Iris->ModelNo;
}




TIris *Iris_New(){
    fprintf(stderr, "sizeof MPI_FINT = %lu\n", sizeof(MPI_Fint));
    TIris *Iris = malloc(sizeof(*Iris));
    return Iris;
}
