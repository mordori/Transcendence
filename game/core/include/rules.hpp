#pragma once

#include <cstdint>

#include "box3d/math_functions.h"
#include "glm/ext/vector_float3.hpp"

namespace core::rules {

struct Goal {
	glm::vec3 center{};
	glm::vec3 halfExtents{};
	uint32_t team{};
};

inline constexpr Goal goals[2]{
	{ .center = { 0.0f, 2.0f, 54.5f }, .halfExtents = { 8.0f, 3.0f, 2.0f }, .team = 0 },
	{ .center = { 0.0f, 2.0f, -54.5f }, .halfExtents = { 8.0f, 3.0f, 2.0f }, .team = 1 },
};

bool isInsideGoal(const glm::vec3& ballPos, const Goal& goal);

inline constexpr glm::vec3 arenaHalfExtents{ 40.0f, 20.0f, 54.5f };
inline constexpr glm::vec3 playerSpawnPoints[6]{ … };

inline constexpr b3Vec3 ballSpawnPoint{ .x = 0.0f, .y = 2.0f, .z = 0.0f };
inline constexpr float goalDistance{ 54.5f };

inline constexpr float startDuration{ 3.0f };
inline constexpr float matchDuration{ 300.0f };
inline constexpr float endDuration{ 60.0f };
}
