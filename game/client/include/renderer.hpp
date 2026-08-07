#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "components/physics.hpp"
#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "webgpu/webgpu_cpp.h"

namespace client::renderer {

struct SharedMesh {
	wgpu::Buffer vertexBuffer{};
	wgpu::Buffer indexBuffer{};
	wgpu::Buffer instanceBuffer{};
	wgpu::BindGroup bindGroup{};

	uint32_t indexCount{};
};

struct SharedMaterial {
	uint32_t pipelineID{};
	wgpu::BindGroup bindGroup{};
};

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

	std::unordered_map<PipelineType, wgpu::RenderPipeline> pipelines;
	wgpu::BindGroupLayout meshBindGroupLayout{};
	wgpu::BindGroupLayout materialBindGroupLayout{};
	wgpu::BindGroupLayout frameBindGroupLayout{};

	wgpu::Buffer frameUniformBuffer{};
	wgpu::BindGroup frameBindGroup{};
	uint32_t maxInstances{ 1024 };
	float aspect{ 1.0f };

	wgpu::Texture depthTexture{};
	wgpu::TextureView depthView{};

	std::unordered_map<uint32_t, SharedMesh> meshes;
	std::unordered_map<uint32_t, std::vector<InstanceData>> instanceBatches;
};

RenderContext& getRenderContext();
void setup(std::function<void(bool success)> onComplete);
void updateCameras(entt::registry& registry, float deltaTime, float alpha);
void prepareScene(entt::registry& registry, float alpha);
void render(entt::registry& registry);

uint32_t createSharedMesh(const MeshData& meshData);
MeshData createMeshCube();
std::optional<MeshData> loadMesh(const std::string& filepath);
}
