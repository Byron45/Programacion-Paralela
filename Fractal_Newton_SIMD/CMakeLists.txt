cmake_minimum_required(VERSION 3.16)

project(NewtonSIMD)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2") #Flags originales

find_package(fmt CONFIG REQUIRED)
find_package(SFML COMPONENTS System Graphics Window CONFIG REQUIRED )

add_executable(newton-simd main.cpp palette.cpp
newton_serial.cpp newton_simd.cpp ) # se listan todos los ejecutables que se deseen

target_link_libraries(newton-simd PRIVATE fmt::fmt
SFML::System
SFML::Graphics
SFML::Window)
