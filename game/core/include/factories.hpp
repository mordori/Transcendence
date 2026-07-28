#pragma once

#include "box3d/id.h"
#include "components/physics.hpp"
#include "entt/entity/fwd.hpp"

namespace core::factories {

void spawnGround(b3WorldId worldId);
entt::entity spawnStadium(entt::registry& registry, b3WorldId worldId, const MeshData& meshData);
entt::entity spawnPlayer(entt::registry& registry, b3WorldId worldId);
entt::entity spawnBall(entt::registry& registry, b3WorldId worldId);
}
