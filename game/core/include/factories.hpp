#pragma once

#include "box3d/id.h"
#include "components/physics.hpp"
#include "entt/entity/fwd.hpp"

namespace core::factories {

void spawn_ground(b3WorldId world_id);
entt::entity spawn_stadium(entt::registry& registry, b3WorldId world_id, const MeshData& meshData);
entt::entity spawn_player(entt::registry& registry, b3WorldId world_id);
entt::entity spawn_ball(entt::registry& registry, b3WorldId world_id);
}
