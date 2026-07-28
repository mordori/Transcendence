#pragma once

#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"

namespace core::systems {

void setupPhysics(entt::registry& registry);
void updatePhysics(entt::registry& registry, float dt);
}
