cmake_minimum_required(VERSION 3.23)

project(http LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

add_library(http INTERFACE)

install(TARGETS http
		EXPORT httpTargets
		LIBRARY DESTINATION http
		ARCHIVE DESTINATION http
		INCLUDES DESTINATION include)

install(DIRECTORY include/ DESTINATION include)

install(EXPORT httpTargets
		FILE httpTargets.cmake
		NAMESPACE http::
		DESTINATION lib/cmake/http)