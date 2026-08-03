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

constexpr Goal goals[2]{
	{ .center = { 0.0f, 2.0f, 54.5f }, .halfExtents = { 8.0f, 3.0f, 2.0f }, .team = 0 },
	{ .center = { 0.0f, 2.0f, -54.5f }, .halfExtents = { 8.0f, 3.0f, 2.0f }, .team = 1 },
};

bool isInsideGoal(const glm::vec3& ballPos, const Goal& goal);

constexpr glm::vec3 arenaHalfExtents{ 40.0f, 20.0f, 54.5f };

// clang-format off
constexpr glm::vec3 playerSpawnPoints[6]{
	{ 0.0f, 0.5f, 35.0f }, { -15.0f, 0.5f, 25.0f }, { 15.0f, 0.5f, 25.0f },
	{ 0.0f, 0.5f, -35.0f }, { -15.0f, 0.5f, -25.0f }, { 15.0f, 0.5f, -25.0f } };
// clang-format on

constexpr glm::vec3 ballSpawnPoint{ 0.0f, 2.0f, 0.0f };
constexpr float goalDistance{ 54.5f };

constexpr float startDuration{ 3.0f };
constexpr float matchDuration{ 60.0f };
constexpr float endDuration{ 5.0f };
}
