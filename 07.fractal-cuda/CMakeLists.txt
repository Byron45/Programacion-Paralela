cmake_minimum_required(VERSION 3.16)

project(FractalCUDA)

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}") #Flags originales
enable_language(CXX CUDA)

find_package(fmt CONFIG REQUIRED)

add_executable(fractal-cuda main.cpp kernel.cu ) # se listan todos los ejecutables que se deseen
target_include_directories(fractal-cuda PRIVATE ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES})

target_link_libraries(fractal-cuda PRIVATE fmt::fmt
)   