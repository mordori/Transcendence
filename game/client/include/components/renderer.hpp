#pragma once

#include <cstdint>

#include "glm/ext/matrix_float4x4.hpp"
#include "webgpu/webgpu_cpp.h"

struct StaticMeshInstance {
	uint32_t meshId{};
	uint32_t materialId{};
};

struct Camera {
	glm::mat4 viewProj{ 1.0f };

	float yaw{};
	float pitch{};
};
