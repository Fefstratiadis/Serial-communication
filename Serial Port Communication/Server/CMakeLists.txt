cmake_minimum_required(VERSION 3.16.3)

# set the project name
project(server)
set(CMAKE_VERBOSE_MAKEFILE on)
include_directories(/usr/include/readline)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
# add the executable
add_executable(server server.c)
target_link_libraries(server readline)