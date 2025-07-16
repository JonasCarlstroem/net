cmake_minimum_required(VERSION 3.23)

project(http LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

add_library(http INTERFACE)

target_link_libraries(http
	INTERFACE types
	INTERFACE utils
	INTERFACE socket
)