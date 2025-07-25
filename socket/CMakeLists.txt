cmake_minimum_required(VERSION 3.23)

project(socket LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

#add_library(socket INTERFACE)
CREATE_LIB(socket INTERFACE)

target_compile_features(socket INTERFACE cxx_std_17)

target_include_directories(socket INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include/net/socket>
)

INSTALL_LIB(socket True net/socket)