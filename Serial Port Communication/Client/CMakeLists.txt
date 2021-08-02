cmake_minimum_required(VERSION 3.16.3)

# set the project name
project(client)
set(CMAKE_VERBOSE_MAKEFILE on)
include_directories(/usr/include/readline)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
# add the executable
add_executable(client cli.c)
target_link_libraries(client readline)