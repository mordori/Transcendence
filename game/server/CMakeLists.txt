set(TARGET_NAME server)

include(FetchContent)

FetchContent_Declare(
	uSockets
	GIT_REPOSITORY https://github.com/uNetworking/uSockets.git
	GIT_TAG        v0.8.8
)

FetchContent_Declare(
	uWebSockets
	GIT_REPOSITORY https://github.com/uNetworking/uWebSockets.git
	GIT_TAG        v20.74.0
)

FetchContent_MakeAvailable(uSockets uWebSockets)

file(GLOB USOCKETS_SOURCES "${usockets_SOURCE_DIR}/src/*.c")

add_library(uSockets_lib STATIC
	${USOCKETS_SOURCES}
	${usockets_SOURCE_DIR}/src/eventing/epoll_kqueue.c
	${usockets_SOURCE_DIR}/src/crypto/openssl.c
	${usockets_SOURCE_DIR}/src/crypto/sni_tree.cpp
)

target_include_directories(uSockets_lib PUBLIC
	${usockets_SOURCE_DIR}/src
)

target_compile_definitions(uSockets_lib PUBLIC LIBUS_USE_OPENSSL)

add_executable(${TARGET_NAME}
	src/main.cpp
	src/network.cpp
)

target_link_libraries(${TARGET_NAME} PRIVATE
	core
	uSockets_lib
	ssl
	crypto
	z
)

target_include_directories(${TARGET_NAME} PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/lib
	${uwebsockets_SOURCE_DIR}/src
	${usockets_SOURCE_DIR}/src
)

target_compile_options(${TARGET_NAME} PRIVATE
	-Wall -Wextra -Werror -pedantic
	-Wno-deprecated-literal-operator -Wno-unused-template
)
