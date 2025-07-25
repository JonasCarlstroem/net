cmake_minimum_required(VERSION 3.23)

project(http LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

#add_library(http INTERFACE)
CREATE_LIB(http INTERFACE)

target_include_directories(http INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include/net/http>
)

INSTALL_LIB(http True net/http)