cmake_minimum_required(VERSION 3.25)
project(Casual C)

set(CMAKE_C_STANDARD 23)


add_executable(Casual main.c)


add_subdirectory(http)
target_link_libraries(Casual http)

target_link_libraries(Casual -lm)