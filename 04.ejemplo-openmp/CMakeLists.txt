cmake_minimum_required(VERSION 3.16)

project(EjemploOpenMP)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp") #Flags originales

find_package(fmt CONFIG REQUIRED)

add_executable(ejemplo-openmp
main.cpp
) # se listan todos los ejecutables que se deseen

target_link_libraries(ejemplo-openmp PRIVATE 
fmt::fmt
) # se listan todas las librerías que se deseen