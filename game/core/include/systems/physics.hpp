#pragma once

#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"

namespace core::systems {

void setup_physics(entt::registry& registry);
void update_physics(entt::registry& registry, float dt);
}
