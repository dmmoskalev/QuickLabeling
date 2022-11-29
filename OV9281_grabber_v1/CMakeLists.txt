cmake_minimum_required(VERSION 3.18.4)
project(grabber)
add_compile_options(
        -Werror
        -Wall
        -lpthread
)
set(CMAKE_CXX_STANDARD 14)
if(NOT CMAKE_CXX_EXTENSIONS)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

add_executable(grabber grabber.cpp)
target_link_libraries(grabber      
         /lib/aarch64-linux-gnu/libpthread.so.0)
link_libraries(/usr/lib/aarch64-linux-gnu/)

