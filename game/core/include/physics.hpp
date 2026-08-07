#pragma once

#include "box3d/math_functions.h"
#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"
#include "glm/ext/vector_float3.hpp"

namespace core::physics {

constexpr glm::vec3 worldRight{ 1.0f, 0.0f, 0.0f };
constexpr glm::vec3 worldUp{ 0.0f, 1.0f, 0.0f };
constexpr glm::vec3 worldForward{ 0.0f, 0.0f, 1.0f };

void setup(entt::registry& registry);
void update(entt::registry& registry, float fixedTimeStep);

constexpr b3Vec3 glmToB3(glm::vec3 vec) {
	return { .x = vec.x, .y = vec.y, .z = vec.z };
}

constexpr glm::vec3 b3ToGlm(b3Vec3 vec) {
	return { vec.x, vec.y, vec.z };
}
}
