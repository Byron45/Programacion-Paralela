cmake_minimum_required(VERSION 3.16)

project(GaussianBlur)

enable_language(CXX CUDA)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2 -fopenmp") #Flags originales

find_package(fmt CONFIG REQUIRED)
find_package(SFML COMPONENTS System Graphics Window CONFIG REQUIRED)

add_executable(gaussian-blur main.cpp blur_openmp.cpp kernel.cu) 
target_include_directories(gaussian-blur PRIVATE ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES})

target_link_libraries(gaussian-blur PRIVATE fmt::fmt
SFML::System
SFML::Graphics
SFML::Window
OpenMP::OpenMP_CXX)
