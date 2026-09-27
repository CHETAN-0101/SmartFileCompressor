cmake_minimum_required(VERSION 3.15)
project(SmartFileCompressor VERSION 2.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Attempt to find system ZLIB; if unavailable, fetch automatically
find_package(ZLIB QUIET)

if(NOT ZLIB_FOUND)
    message(STATUS "Local ZLIB package not found. Fetching ZLIB 1.3.1 automatically via CMake FetchContent...")
    include(FetchContent)
    FetchContent_Declare(
        zlib
        GIT_REPOSITORY https://github.com/madler/zlib.git
        GIT_TAG        v1.3.1
    )
    FetchContent_MakeAvailable(zlib)
    set(ZLIB_LIBRARIES zlibstatic)
    set(ZLIB_INCLUDE_DIRS ${zlib_SOURCE_DIR} ${zlib_BINARY_DIR})
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${ZLIB_INCLUDE_DIRS}
)

# Core OS sources conditionally selected based on target OS
if (WIN32)
    set(OS_SOURCES src/os/process_monitor_win.cpp src/os/scheduler.cpp)
else()
    set(OS_SOURCES src/os/process_monitor_nix.cpp src/os/scheduler.cpp)
endif()

# Core engine library
add_library(sfc_engine
    src/core/mmap_file.cpp
    src/core/compressor.cpp
    src/core/decompressor.cpp
    src/core/thread_pool.cpp
    src/ipc/ipc.cpp
    ${OS_SOURCES}
)

target_link_libraries(sfc_engine PRIVATE ${ZLIB_LIBRARIES})

# CLI Executable
add_executable(sfc src/cli/main_cli.cpp)
target_link_libraries(sfc PRIVATE sfc_engine ${ZLIB_LIBRARIES})

# Background Daemon Executable
add_executable(sfcd src/daemon/main_daemon.cpp)
target_link_libraries(sfcd PRIVATE sfc_engine ${ZLIB_LIBRARIES})

# Unit Test Executable
add_executable(test_sfc tests/test_sfc.cpp)
target_link_libraries(test_sfc PRIVATE sfc_engine ${ZLIB_LIBRARIES})
