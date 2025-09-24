cmake_minimum_required(VERSION 3.10)
project(SmartFileCompressor)

find_package(ZLIB REQUIRED)

include_directories(${ZLIB_INCLUDE_DIRS})

add_library(compress_module src/compressor.cpp)
add_library(decompress_module src/decompressor.cpp)

add_executable(sfc src/viewer.cpp)
target_link_libraries(sfc compress_module ${ZLIB_LIBRARIES})

add_executable(sfc_viewer src/sfc_viewer.cpp)
target_link_libraries(sfc_viewer decompress_module ${ZLIB_LIBRARIES})
