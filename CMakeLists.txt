cmake_minimum_required(VERSION 3.23)

project(net LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(socket)
add_subdirectory(server)

add_library(net INTERFACE)

target_compile_features(net INTERFACE cxx_std_17)

target_include_directories(net INTERFACE 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>
)

target_link_libraries(net 
	INTERFACE socket
	INTERFACE server
)

install(TARGETS net
		EXPORT libTargets
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
		RUNTIME DESTINATION bin
		INCLUDES DESTINATION include)

install(DIRECTORY include/ DESTINATION include)

set(LIB_TARGETS net PARENT_SCOPE)