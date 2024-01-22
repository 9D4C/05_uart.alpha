cmake_minimum_required(VERSION 3.0)

project(uart)
set (EXECUTABLE_OUTPUT_PATH ../bin)

include_directories(./inc)
aux_source_directory(./src SRC)
add_executable(uart ${SRC})

