#pragma once

#include <cstdint>
#include <vector>

#include "glm/ext/vector_float3.hpp"
#include "glm/fwd.hpp"
#include "glm/gtc/quaternion.hpp"

struct NetworkId {
	uint32_t id{};
};

struct EntityState {
	uint32_t id{};
	glm::vec3 pos{};
	glm::quat rot{ 1.0f, 0.0f, 0.0f, 0.0f };
};

struct Snapshot {
	uint32_t tick{};
	std::vector<EntityState> players;
	std::vector<EntityState> balls;
};
