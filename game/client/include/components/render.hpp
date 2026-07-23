#pragma once

#include <cstdint>

#include "glm/ext/matrix_float4x4.hpp"
#include "glm/glm.hpp"
#include "webgpu/webgpu_cpp.h"

struct Transform {
	glm::mat4 mvp;
};

struct Renderable {
	wgpu::Buffer vertexBuffer{};
	wgpu::Buffer indexBuffer{};
	uint32_t indexCount{};
	wgpu::Buffer uniformBuffer{};
	wgpu::BindGroup bindGroup{};
};
