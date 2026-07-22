#pragma once

#include <entt/entt.hpp>

#include "entt/entity/fwd.hpp"

namespace client::systems {
void setup_input(entt::registry& registry, entt::entity local_player);
}
