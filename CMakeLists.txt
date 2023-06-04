cmake_minimum_required(VERSION 3.16)
project(Projekt_02 C)

set(CMAKE_C_STANDARD 99)

add_executable(Projekt_02 ipkcpd.c)

target_link_libraries(Projekt_02 pthread)
