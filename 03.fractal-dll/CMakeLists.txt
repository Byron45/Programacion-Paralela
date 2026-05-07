cmake_minimum_required(VERSION 3.16)

project(FractalJulia)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2") #Flags originales

find_package(fmt CONFIG REQUIRED)

add_library(fractal-dll SHARED
palette.cpp
fractal_simd.cpp ) # se listan todos los ejecutables que se deseen

target_link_libraries(fractal-dll PRIVATE 
fmt::fmt
)   