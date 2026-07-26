#pragma once

#include <functional>

#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"
#include "webgpu/webgpu_cpp.h"

namespace client::systems {
struct RenderContext {
	wgpu::Instance instance{};
	wgpu::Adapter adapter{};
	wgpu::Device device{};
	wgpu::Surface surface{};
	wgpu::SurfaceConfiguration config{};
	wgpu::Queue queue{};
	wgpu::RenderPipeline pipeline{};
	wgpu::BindGroupLayout bindGroupLayout{};
	float aspect{};
};

RenderContext& get_render_context();
void init_webgpu(std::function<void(bool success)> init_status);
void render(entt::registry& registry);
}
