cmake_minimum_required(VERSION 3.16)

project(Ejemplo01)

find_package(fmt CONFIG REQUIRED)

add_executable(ejemplo01 main.cpp) # se listan todos los ejecutables que se deseen

target_link_libraries(ejemplo01 PRIVATE fmt::fmt)