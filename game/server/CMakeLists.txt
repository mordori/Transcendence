set(TARGET_NAME server)

add_executable(${TARGET_NAME}
	src/main.cpp
	src/network.cpp
)

target_link_libraries(${TARGET_NAME} PRIVATE
	core
)

target_include_directories(${TARGET_NAME} PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/lib
)

target_compile_options(${TARGET_NAME} PRIVATE
	-Wall -Wextra -Werror -pedantic
	-Wno-deprecated-literal-operator -Wno-unused-template
)
