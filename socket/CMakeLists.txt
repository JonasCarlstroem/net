cmake_minimum_required(VERSION 3.23)

project(socket LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(socket INTERFACE)

target_precompile_headers(socket INTERFACE pch.h)