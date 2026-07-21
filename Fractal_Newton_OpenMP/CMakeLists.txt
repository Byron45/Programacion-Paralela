cmake_minimum_required(VERSION 3.16)

project(NewtonOpenMP)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp") #Flags originales

find_package(fmt CONFIG REQUIRED)
find_package(SFML COMPONENTS System Graphics Window CONFIG REQUIRED )

add_executable(newton-openmp main.cpp palette.cpp
newton_serial.cpp newton_openmp.cpp ) # se listan todos los ejecutables que se deseen

target_link_libraries(newton-openmp PRIVATE fmt::fmt
SFML::System
SFML::Graphics
SFML::Window)
