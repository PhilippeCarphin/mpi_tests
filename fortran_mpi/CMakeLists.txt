cmake_minimum_required(VERSION 3.16)

project(c_fortran_mpi C Fortran)

find_package(MPI REQUIRED)

add_library(c_iris SHARED iris.c)
add_library(fortran_iris SHARED iris.f90)
add_executable(test_fortran_iris main.f90)

target_link_libraries(c_iris PUBLIC MPI::MPI_C)
target_link_libraries(fortran_iris PUBLIC MPI::MPI_Fortran)
target_link_libraries(fortran_iris PUBLIC c_iris)
target_link_libraries(test_fortran_iris PUBLIC fortran_iris)

