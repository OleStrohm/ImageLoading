cmake_minimum_required(VERSION 3.8)
project(ImageLoading)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp image.cpp image.h bitarray.cpp bitarray.h node.h tree.cpp tree.h)
add_executable(ImageLoading ${SOURCE_FILES})
