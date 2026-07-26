#pragma once

#include "box3d/id.h"
#include "entt/entity/fwd.hpp"

namespace core::factories {
void spawn_ground(b3WorldId world_id);
entt::entity spawn_player(entt::registry& registry, b3WorldId world_id);
}
