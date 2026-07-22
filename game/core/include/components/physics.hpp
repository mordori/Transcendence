#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct Transform {
	glm::vec3 position{};
	glm::quat rotation{ 1.0f, 0.0f, 0.0f, 0.0f };
};
