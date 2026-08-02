#pragma once

#include "components/audio.hpp"
#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"

namespace client::audio {

void setup(entt::registry& registry);

void attachEngine(entt::registry& registry, entt::entity entity);

void update(entt::registry& registry, float deltaTime);
}
