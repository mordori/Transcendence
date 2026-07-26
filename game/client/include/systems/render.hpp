#pragma once

#include <cstdint>
#include <functional>
#include <unordered_map>

#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "webgpu/webgpu_cpp.h"

namespace client::systems {

struct alignas(16) InstanceData {
	glm::mat4 model;
};

enum class PipelineType : uint32_t {
	Opaque = 0,
	Unlit = 1,
	Transparent = 2,
};

struct RenderContext {
	wgpu::Instance instance{};
	wgpu::Adapter adapter{};
	wgpu::Device device{};
	wgpu::Surface surface{};
	wgpu::SurfaceConfiguration config{};
	wgpu::Queue queue{};

	std::unordered_map<uint32_t, wgpu::RenderPipeline> pipelines;
	wgpu::BindGroupLayout globalBindGroupLayout{};
	wgpu::BindGroupLayout materialBindGroupLayout{};

	wgpu::Buffer cameraBuffer{};
	wgpu::Buffer instanceBuffer{};
	wgpu::BindGroup globalBindGroup{};
	float aspect{ 1.0f };

	wgpu::RenderPipeline pipeline{};
	wgpu::BindGroupLayout bindGroupLayout{};
};

RenderContext& get_render_context();
void init_webgpu(std::function<void(bool success)> init_status);
void render(entt::registry& registry);
}
