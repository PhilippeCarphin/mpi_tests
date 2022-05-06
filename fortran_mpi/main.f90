program scatter_demo
    use, intrinsic :: ISO_C_BINDING
    use mpi
    use iris
    implicit none

    integer :: ierror
    integer :: rank
    type(C_PTR) :: my_iris
    integer :: iris_init_value
    real(C_FLOAT), dimension(100,100) :: f_array

    my_iris = iris_new()
    call MPI_INIT(ierror)
    call MPI_COMM_RANK(MPI_COMM_WORLD, rank, ierror)
    iris_init_value = iris_model_init(my_iris, 1)

    if (rank == 0) then
        write (*,*) "iris_init_value = ", iris_init_value
    endif

    iris_init_value = firis_model_init(my_iris, 1)

    if (rank == 0) then
        write (*,*) "iris_init_value = ", iris_init_value
        write (*,*) "ModelNo = ", iris_get_modelno(my_iris)
    endif


    call MPI_FINALIZE(ierror)

end program
