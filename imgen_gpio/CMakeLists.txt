cmake_minimum_required(VERSION 3.18.4)
project(imgen)

add_compile_options(
    -Werror
    -Wall
)

set(CMAKE_CXX_STANDARD 14)
if(NOT CMAKE_CXX_EXTENSIONS)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

add_executable(imgen imgen.cpp)
target_link_libraries(imgen     
        /lib/aarch64-linux-gnu/libpthread.so.0)
link_libraries(/usr/lib/aarch64-linux-gnu/)

