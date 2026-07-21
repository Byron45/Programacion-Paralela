cmake_minimum_required(VERSION 3.16)

project(NewtonMPI)

find_package(fmt CONFIG REQUIRED)
find_package(SFML COMPONENTS System Graphics Window CONFIG REQUIRED )
find_package(Freetype REQUIRED)

set(MPI_ROOT "C://oneAPI//mpi//latest")
set(MPI_INCLUDE "${MPI_ROOT}/include")
set(MPI_LIB "${MPI_ROOT}/lib")

include_directories(${MPI_INCLUDE})

add_executable(newton-mpi main.cpp newton_mpi.cpp palette.cpp draw_text.cpp
) 

target_link_libraries(newton-mpi PRIVATE fmt::fmt
SFML::System
SFML::Graphics
SFML::Window
Freetype::Freetype
${MPI_LIB}/impi.lib
)   
