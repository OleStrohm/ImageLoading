cmake_minimum_required(VERSION 3.8)
project(ImageLoading)

set(CMAKE_CXX_STANDARD 11)


file(GLOB_RECURSE FREEIMAGE_SOURCE zlib-master/*.c)
file(GLOB_RECURSE FREEIMAGE_HEADERS zlib-master/*.h)

add_library(FreeImageLib STATIC ${FREEIMAGE_SOURCE} ${FREEIMAGE_HEADERS})

set(SOURCE_FILES main.cpp image.cpp image.h bitarray.cpp bitarray.h node.cpp node.h tree.cpp tree.h)
add_executable(ImageLoading ${SOURCE_FILES})

target_link_libraries(ImageLoading FreeImageLib)
