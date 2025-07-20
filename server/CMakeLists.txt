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

install(TARGETS server
		EXPORT serverTargets
		LIBRARY DESTINATION server
		ARCHIVE DESTINATION server
		INCLUDES DESTINATION include)

install(DIRECTORY include/ DESTINATION include)

install(EXPORT serverTargets
		FILE serverTargets.cmake
		NAMESPACE server::
		DESTINATION lib/cmake/server)