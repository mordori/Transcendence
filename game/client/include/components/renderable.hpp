#pragma once

#include <cstdint>

#include "webgpu/webgpu_cpp.h"

struct Renderable {
	wgpu::Buffer vertexBuffer{};
	wgpu::Buffer indexBuffer{};
	uint32_t indexCount{};
	wgpu::Buffer uniformBuffer{};
	wgpu::BindGroup bindGroup{};
};

struct MeshComponent {
	wgpu::Buffer vertexBuffer{};
	wgpu::Buffer indexBuffer{};
	uint32_t indexCount{};
};

struct MaterialComponent {
	uint32_t pipelineID{};
	wgpu::BindGroup bindGroup{};
};
