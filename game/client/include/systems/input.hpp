#pragma once

#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"

namespace client::systems {
void setup_input(entt::registry& registry, entt::entity local_player);
}
