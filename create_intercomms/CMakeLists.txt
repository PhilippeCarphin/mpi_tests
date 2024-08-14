cmake_minimum_required(VERSION 3.20)
project(scatter_gather_V C Fortran)

find_package(MPI REQUIRED)

set(CMAKE_C_COMPILER ${MPI_C_COMPILER})
set(CMAKE_Fortran_COMPILER ${MPI_Fortran_COMPILER})

add_library(coupler coupler.c)
add_executable(coupler_main coupler_main.c)
add_executable(model model.c)
target_link_libraries(model PRIVATE coupler)
target_link_libraries(coupler_main PRIVATE coupler)

include(CTest)
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --verbose)

add_test(NAME run COMMAND mpirun -np 1 $<TARGET_FILE:coupler_main> 3 : -np 4 $<TARGET_FILE:model> : -np 8 $<TARGET_FILE:model> : -np 12 $<TARGET_FILE:model>)
# add_test(NAME run COMMAND mpirun -np 1 $<TARGET_FILE:coupler_main> 1 : -np 4 $<TARGET_FILE:model>)
add_dependencies(check coupler_main model)
