cmake_minimum_required(VERSION 3.23)

project(socket LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

add_library(socket INTERFACE)

target_compile_features(socket INTERFACE cxx_std_17)
#target_link_libraries(socket INTERFACE ws2_32)
#target_precompile_headers(socket PRIVATE pch.hpp)