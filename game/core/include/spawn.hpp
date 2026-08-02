#pragma once

#include <cstdint>

#include "box3d/id.h"
#include "components/physics.hpp"
#include "entt/entity/fwd.hpp"

namespace core::spawn {

void ground(b3WorldId worldId);
entt::entity stadium(
	entt::registry& registry, b3WorldId worldId, const MeshData& meshData, uint32_t category, uint32_t mask);
entt::entity player(entt::registry& registry, b3WorldId worldId);
entt::entity ball(entt::registry& registry, b3WorldId worldId);
}
