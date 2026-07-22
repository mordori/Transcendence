set(TARGET_NAME client)

add_executable(${TARGET_NAME}
	src/main.cpp
	src/systems/render.cpp
	src/systems/input.cpp
)

target_link_libraries(${TARGET_NAME} PRIVATE core)

target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_options(${TARGET_NAME} PRIVATE
	-Wall -Wextra -Werror -pedantic
	-Wno-deprecated-literal-operator -Wno-unused-template
)

if (EMSCRIPTEN)
	set_target_properties(${TARGET_NAME} PROPERTIES SUFFIX ".html")

	target_compile_options(${TARGET_NAME} PRIVATE "--use-port=emdawnwebgpu")

	target_link_options(${TARGET_NAME} PRIVATE
		"-sWASM=1"
		"-sALLOW_MEMORY_GROWTH=1"
		"--use-port=emdawnwebgpu"
	)
endif()
