cmake_minimum_required(VERSION 3.23)

project(server LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
	add_compile_options(/Zc:__cplusplus)
endif()

add_subdirectory(http)
add_subdirectory(dns)

add_library(server INTERFACE)

target_link_libraries(server INTERFACE http)