cmake_minimum_required(VERSION 3.16)

project(Ejemplo02)

find_package(fmt CONFIG REQUIRED)
find_package(SFML COMPONENTS System Graphics Window CONFIG REQUIRED )

add_executable(ejemplo02 main.cpp) # se listan todos los ejecutables que se deseen

target_link_libraries(ejemplo02 PRIVATE fmt::fmt
SFML::System
SFML::Graphics
SFML::Window)