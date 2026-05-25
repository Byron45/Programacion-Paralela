cmake_minimum_required(VERSION 3.16)

project(TrabajoClase)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2 -fopenmp") #Flags originales

find_package(fmt CONFIG REQUIRED)
find_package(SFML COMPONENTS System Graphics Window CONFIG REQUIRED )

add_executable(TrabajoClase main.cpp ) # se listan todos los ejecutables que se deseen

target_link_libraries(TrabajoClase PRIVATE fmt::fmt
SFML::System
SFML::Graphics
SFML::Window)   