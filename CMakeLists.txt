cmake_minimum_required(VERSION 3.14.0)

project(cpp-dev-test LANGUAGES CXX C)
set(CMAKE_INSTALL_PREFIX "${cpp-dev-test_SOURCE_DIR}/bin")
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


add_compile_options(-Wall)

include(FetchContent)

# Thread library
find_package(Threads)

# External dependencies
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")
include(asio)
include(flatBuffers)

add_subdirectory(src)
