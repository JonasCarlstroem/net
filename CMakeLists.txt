cmake_minimum_required(VERSION 3.23)

project(net LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(socket)
add_subdirectory(http)

#add_library(net INTERFACE)
CREATE_LIB(net INTERFACE)

target_compile_features(net INTERFACE cxx_std_17)

#target_include_directories(net INTERFACE 
#	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#	$<INSTALL_INTERFACE:include>
#)

target_link_libraries(net 
	INTERFACE socket
	INTERFACE http
)

INSTALL_LIB(net False net)