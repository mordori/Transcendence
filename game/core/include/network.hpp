#pragma once

#include <cstdint>
#include <glaze/core/refl.hpp>
#include <glaze/glaze.hpp>
#include <glaze/json/write.hpp>
#include <string>

#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"

namespace core::network {

std::string buildSnapshot(entt::registry& registry, uint32_t currentTick);
}
