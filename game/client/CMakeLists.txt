add_executable(game-client
	src/main.cpp
	src/systems/render.cpp
)

target_link_libraries(game-client PRIVATE core)

target_include_directories(game-client PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_options(game-client PRIVATE -Wall -Wextra -Werror -pedantic)
