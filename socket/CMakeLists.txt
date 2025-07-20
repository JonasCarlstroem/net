cmake_minimum_required(VERSION 3.23)

project(socket LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

add_library(socket INTERFACE)

target_compile_features(socket INTERFACE cxx_std_17)

install(TARGETS socket
		EXPORT socketTargets
		LIBRARY DESTINATION socket
		ARCHIVE DESTINATION socket
		INCLUDES DESTINATION include)

install(DIRECTORY include/ DESTINATION include)

install(EXPORT socketTargets
		FILE socketTargets.cmake
		NAMESPACE socket::
		DESTINATION lib/cmake/socket)