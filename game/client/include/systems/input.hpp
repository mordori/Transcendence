#pragma once

#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"

namespace client::systems {

void setupInput(entt::registry& registry, entt::entity player);
}
