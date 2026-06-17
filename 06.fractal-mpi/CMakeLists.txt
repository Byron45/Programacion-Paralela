cmake_minimum_required(VERSION 3.16)

project(FactalMPI)

find_package(fmt CONFIG REQUIRED)
find_package(SFML COMPONENTS System Graphics Window CONFIG REQUIRED )
find_package(Freetype REQUIRED)

# Cofig MPI
set(MPI_ROOT "C://oneAPI//mpi//latest")
set(MPI_INCLUDE "${MPI_ROOT}/include")
set(MPI_LIB "${MPI_ROOT}/lib")

include_directories(${MPI_INCLUDE})

add_executable(fractal-mpi main.cpp fractal_mpi.cpp palette.cpp draw_text.cpp
) # se listan todos los ejecutables que se deseen

target_link_libraries(fractal-mpi PRIVATE fmt::fmt
SFML::System
SFML::Graphics
SFML::Window
Freetype::Freetype
${MPI_LIB}/impi.lib
)   

