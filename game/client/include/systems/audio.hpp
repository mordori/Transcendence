#pragma once

#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"
#include "components/audio.hpp"

namespace client::systems {
    void    setupAudio(entt::registry& registry);
    void    updateAudio(entt::registry& registry, float dt);
}