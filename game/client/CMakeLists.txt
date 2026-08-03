set(TARGET_NAME client)

add_executable(${TARGET_NAME}
	src/main.cpp
	src/input.cpp
	src/renderer.cpp
	src/renderable.cpp
	src/audio.cpp
)

target_link_libraries(${TARGET_NAME} PRIVATE
	core
	${CMAKE_CURRENT_SOURCE_DIR}/lib/fmod/api/studio/lib/w32/fmodstudioL_wasm.a
)

target_include_directories(${TARGET_NAME} PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/lib
)

target_include_directories(${TARGET_NAME} SYSTEM PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/lib/fmod/api/core/inc
	${CMAKE_CURRENT_SOURCE_DIR}/lib/fmod/api/studio/inc
)

target_compile_options(${TARGET_NAME} PRIVATE
	-Wall -Wextra -Werror -pedantic
	-Wno-deprecated-literal-operator -Wno-unused-template
)

set_target_properties(${TARGET_NAME} PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../frontend/src/wasm"
	SUFFIX ".js"
)

target_compile_options(${TARGET_NAME} PRIVATE "--use-port=emdawnwebgpu")

target_link_options(${TARGET_NAME} PRIVATE
	"-sWASM=1"
	"-sEXIT_RUNTIME=1"
	"-sALLOW_MEMORY_GROWTH=1"
	"--use-port=emdawnwebgpu"
	"-sMODULARIZE=1"
	"-sEXPORT_ES6=1"
	"-sEXPORT_NAME='createEngine'"
	"-sEXPORTED_RUNTIME_METHODS=[\"FS\",\"ccall\",\"cwrap\",\"setValue\",\"getValue\"]"
)
