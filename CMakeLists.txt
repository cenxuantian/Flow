cmake_minimum_required(VERSION 3.5.0)
project(flow VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")

option(BUILD_TEST ON)
option(BUILD_BENCHMARK ON)
option(BUILD_EXAMPLE ON)
option(BUILD_EXECUTOR ON)

include_directories(flow/include)
include(test/CMakeLists.txt)
include(flow/executor/CmakeLists.txt)


