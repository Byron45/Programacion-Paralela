cmake_minimum_required(VERSION 3.16)

project(FractalNEWTON)

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}") #Flags originales
enable_language(CXX CUDA)

find_package(fmt CONFIG REQUIRED)
find_package(SFML COMPONENTS System Graphics Window CONFIG REQUIRED )



add_executable(fractal-newton main.cpp kernel.cu palette.cpp ) # se listan todos los ejecutables que se deseen
target_include_directories(fractal-newton PRIVATE ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES})

target_link_libraries(fractal-newton PRIVATE fmt::fmt 
SFML::System
SFML::Graphics
SFML::Window)   