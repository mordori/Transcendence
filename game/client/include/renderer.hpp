#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>

#include "components/physics.hpp"
#include "components/renderer.hpp"
#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "webgpu/webgpu_cpp.h"

namespace client::renderer {

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

	wgpu::Texture depthTexture{};
	wgpu::TextureView depthView{};

	wgpu::RenderPipeline pipeline{};
	wgpu::BindGroupLayout bindGroupLayout{};
};

RenderContext& getRenderContext();
void setup(std::function<void(bool success)> onComplete);
void render(entt::registry& registry, float deltaTime, float alpha);

MeshData createMeshCube();
std::optional<MeshData> loadMesh(const std::string& filepath);
Renderable createRenderable(const MeshData& mesh);
}
