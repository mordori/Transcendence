#pragma once

#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"

namespace client::input {

struct LocalPlayer {
	entt::entity id;
};

void setup(entt::registry& registry, entt::entity player);
}
