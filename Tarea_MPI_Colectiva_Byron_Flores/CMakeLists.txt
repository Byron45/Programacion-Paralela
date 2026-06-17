cmake_minimum_required(VERSION 3.16)

project(EjemploMPI)

find_package(fmt CONFIG REQUIRED)

# Cofig MPI
set(MPI_ROOT "C://oneAPI//mpi//latest")
set(MPI_INCLUDE "${MPI_ROOT}/include")
set(MPI_LIB "${MPI_ROOT}/lib")

include_directories(${MPI_INCLUDE})

add_executable(ejemplo-mpi main.cpp 
) # se listan todos los ejecutables que se deseen

target_link_libraries(ejemplo-mpi PRIVATE fmt::fmt
${MPI_LIB}/impi.lib
)   

add_executable(matrices-mult matrices_mult.cpp
)

add_executable(matrices-mult-colectiva matrices_mult_colectiva.cpp
)

target_link_libraries(matrices-mult PRIVATE fmt::fmt
${MPI_LIB}/impi.lib
)

target_link_libraries(matrices-mult-colectiva PRIVATE fmt::fmt
${MPI_LIB}/impi.lib
) 