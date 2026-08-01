#pragma once

#include "components/audio.hpp"
#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"

namespace client::audio {

void setup(entt::registry& registry);
void update(entt::registry& registry, float deltaTime);
}
