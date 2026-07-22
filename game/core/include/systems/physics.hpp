#pragma once

#include <entt/entt.hpp>

#include "entt/entity/fwd.hpp"

namespace core::systems {
void update_physics(entt::registry& registry, float dt);
}
