#pragma once

#include <cstdint>
#include <vector>

#include "webgpu/webgpu_cpp.h"

struct Renderable {
	wgpu::Buffer vertexBuffer{};
	wgpu::Buffer indexBuffer{};
	uint32_t indexCount{};
	wgpu::Buffer uniformBuffer{};
	wgpu::BindGroup bindGroup{};
};

struct MeshData {
	std::vector<float> vertices;
	std::vector<uint16_t> indices;
};
