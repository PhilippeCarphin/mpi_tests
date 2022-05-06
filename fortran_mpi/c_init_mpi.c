// https://github.com/LadaF/PoisFFT/blob/master/src/f_mpi_comm_c2f.c

int Iris_Init(TIris *Iris, int ModelNb){
    // Represents something that does mpi_init, splits communicators and stuff
    // and stores two communicators in a struct.
    //
    // Here just do mpi_init and return the communicator
    //

    return 7;
}

TIris *Iris_New(){
    TIris *Iris = malloc(sizeof(*Iris));
    return Iris;
}
