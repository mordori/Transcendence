#pragma once

#include "entt/entity/fwd.hpp"

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
}
