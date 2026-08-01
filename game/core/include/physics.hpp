#pragma once

#include "box3d/math_functions.h"
#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"
#include "glm/ext/vector_float3.hpp"

namespace core::physics {

void setup(entt::registry& registry);
void update(entt::registry& registry, float fixedTimeStep);

inline b3Vec3 glmToB3(glm::vec3 vec) {
	return { .x = vec.x, .y = vec.y, .z = vec.z };
}

inline glm::vec3 b3ToGlm(b3Vec3 vec) {
	return { vec.x, vec.y, vec.z };
}
}
