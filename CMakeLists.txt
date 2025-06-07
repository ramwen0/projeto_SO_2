cmake_minimum_required(VERSION 3.26)
project(T2)

set(CMAKE_C_STANDARD 11)

include_directories(include)

add_executable(T2
        main.c memory_sim.c inputs_part1.c)