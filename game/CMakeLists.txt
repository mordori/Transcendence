cmake_minimum_required(VERSION 3.20)
project(Game)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(core)
if (EMSCRIPTEN)
	add_subdirectory(client)
else()
	add_subdirectory(server)
endif()
# add_subdirectory(ai)
