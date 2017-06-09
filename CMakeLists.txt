cmake_minimum_required(VERSION 3.5)
project(CacheFS)

set(CMAKE_CXX_STANDARD 11)

SET(GCC_COVERAGE_COMPILE_FLAGS "-Wall -Wextra -std=c++11")
SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES CacheFS.h CacheFS.cpp Cache.h Cache.cpp Main.cpp)
add_executable(${SOURCE_FILES} Main)

