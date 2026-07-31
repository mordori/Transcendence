#pragma once

#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"

namespace core::physics {

void setup(entt::registry& registry);
void update(entt::registry& registry, float fixedTimeStep);
}
