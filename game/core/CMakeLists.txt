set(TARGET_NAME core)

include(FetchContent)

FetchContent_Declare(
	glm
	GIT_REPOSITORY	https://github.com/g-truc/glm.git
	GIT_TAG			1.0.1
)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(
	box3d
	GIT_REPOSITORY	https://github.com/erincatto/box3d.git
	GIT_TAG			main
)
FetchContent_MakeAvailable(box3d)

FetchContent_Declare(
	entt
	GIT_REPOSITORY	https://github.com/skypjack/entt.git
	GIT_TAG			v3.13.1
)
FetchContent_MakeAvailable(entt)

FetchContent_Declare(
	glaze
	GIT_REPOSITORY	https://github.com/stephenberry/glaze.git
	GIT_TAG			v3.4.0
)
FetchContent_MakeAvailable(glaze)

add_library(${TARGET_NAME} STATIC
	src/utils/files.cpp
	src/match.cpp
	src/network.cpp
	src/physics.cpp
	src/spawn.cpp
)

target_link_libraries(${TARGET_NAME} PUBLIC
	glm::glm
	box3d::box3d
	EnTT::EnTT
	glaze::glaze
)

target_include_directories(${TARGET_NAME} PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/lib
)

target_compile_options(${TARGET_NAME} PRIVATE
	-Wall -Wextra -Werror -pedantic
	-Wno-deprecated-literal-operator -Wno-unused-template
)
