cmake_minimum_required(VERSION 3.18.4)
project(ql)
add_compile_options(
        -Werror
        -Wall
        -lpthread
)
set(CMAKE_CXX_STANDARD 14)
if(NOT CMAKE_CXX_EXTENSIONS)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()
find_package(OpenCV REQUIRED)
include_directories(/usr/local/include/opencv4)
add_executable(ql ql.cpp)
target_link_libraries(ql
       /usr/local/lib/aarch64-linux-gnu/libopencv_core.so
        /usr/local/lib/aarch64-linux-gnu/libopencv_imgcodecs.so
        /usr/local/lib/aarch64-linux-gnu/libopencv_imgproc.so.4.6.0
        /usr/local/lib/aarch64-linux-gnu/libopencv_highgui.so.4.6.0
        /lib/aarch64-linux-gnu/libpthread.so.0)
link_libraries(/usr/lib/aarch64-linux-gnu/)

