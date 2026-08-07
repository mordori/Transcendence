#pragma once

#include "entt/entity/fwd.hpp"
#include "glm/ext/vector_float3.hpp"
#include "rules.hpp"

namespace core::match {

enum class State {
	LOBBY,
	START,
	ONGOING,
	END,
};

void setup(entt::registry& registry);
void reset(entt::registry& registry);
void update(entt::registry& registry, float fixedTimeStep);

bool isInsideGoal(const glm::vec3& ballPos, const core::rules::Goal& goal);
}
