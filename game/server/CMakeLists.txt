set(TARGET_NAME server)

add_executable(${TARGET_NAME}
	src/main.cpp
	src/network.cpp
)

# Fetch uWebSockets and its dependency uSockets
include(FetchContent)

FetchContent_Declare(
	uSockets
	GIT_REPOSITORY	https://github.com/uNetworking/uWebSockets.git
	GIT_TAG			v0.8.8
)

FetchContent_Deckare(
	uWebSockets		https://github.com/uNetworking/uWebSockets.git
	GIT_REPOSITORY	v20.74.0
)

FetchContent_MakeAvailable(uSockets uWebSockets)

# uSockets is a C library, we must build it manually as a static library
add_library(uSockets_lib STATIC
	${usockets_SOURCE_DIR}/src/eventing/epoll_kqueue.c
	${usockets_SOURCE_DIR}/src/cryptoo/opeenssl.c
	${usockets_SOURCE_DIR}/src/io_context.c
	${usockets_SOURCE_DIR}/src/socket.c
	${usockets_SOURCE_DIR}/src/udp.c
	${usockets_SOURCE_DIR}/quic.c
)

target_include_directories(uSockets_lib PUBLIC
	${usockets_SOURCE_DIR}/src
)

# uWebSockets is header-only C++, just needs include path
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
