#include <array>
#include <cstdint>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "components/renderer.hpp"

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"
#include "components/physics.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "renderer.hpp"
#include "webgpu/webgpu_cpp.h"

namespace client::renderer {

uint32_t createSharedMesh(const MeshData& meshData) {
	auto& ctx = getRenderContext();
	static uint32_t meshId{ 1 };

	SharedMesh mesh{};
	mesh.indexCount = meshData.indices.size();

	wgpu::BufferDescriptor vbo{};
	vbo.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
	vbo.size = sizeof(float) * meshData.vertices.size();
	mesh.vertexBuffer = ctx.device.CreateBuffer(&vbo);
	ctx.queue.WriteBuffer(mesh.vertexBuffer, 0, meshData.vertices.data(), vbo.size);

	wgpu::BufferDescriptor ibo{};
	ibo.usage = wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst;
	ibo.size = sizeof(uint32_t) * meshData.indices.size();
	mesh.indexBuffer = ctx.device.CreateBuffer(&ibo);
	ctx.queue.WriteBuffer(mesh.indexBuffer, 0, meshData.indices.data(), ibo.size);

	wgpu::BufferDescriptor instBufDesc{};
	instBufDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
	instBufDesc.size = sizeof(InstanceData) * ctx.maxInstances;
	mesh.instanceBuffer = ctx.device.CreateBuffer(&instBufDesc);

	wgpu::BindGroupEntry bgEntry{};
	bgEntry.binding = 0;
	bgEntry.buffer = mesh.instanceBuffer;
	bgEntry.size = sizeof(InstanceData) * ctx.maxInstances;

	wgpu::BindGroupDescriptor bgDesc{};
	bgDesc.layout = ctx.meshBindGroupLayout;
	bgDesc.entryCount = 1;
	bgDesc.entries = &bgEntry;
	mesh.bindGroup = ctx.device.CreateBindGroup(&bgDesc);

	uint32_t id{ meshId++ };
	ctx.meshes[id] = mesh;
	return id;
}

MeshData createMeshCube() {
	MeshData meshData{};
	// clang-format off
	meshData.vertices = {
		-1, -1,  1,   1, 0, 0,
		 1, -1,  1,   0, 1, 0,
		 1,  1,  1,   0, 0, 1,
		-1,  1,  1,   1, 1, 0,
		-1, -1, -1,   1, 0, 1,
		 1, -1, -1,   0, 1, 1,
		 1,  1, -1,   1, 1, 1,
		-1,  1, -1,   0, 0, 0
	};
	meshData.indices = {
		0,1,2, 2,3,0, 1,5,6, 6,2,1, 5,4,7, 7,6,5,
		4,0,3, 3,7,4, 3,2,6, 6,7,3, 4,5,1, 1,0,4
	};
	// clang-format on
	return meshData;
}

std::optional<MeshData> loadMesh(const std::string& filepath) {
	MeshData meshData{};
	cgltf_options opts{};
	cgltf_data* data{ nullptr };

	cgltf_result result{ cgltf_parse_file(&opts, filepath.data(), &data) };
	if (result != cgltf_result_success) {
		std::cerr << "[cgltf] Failed to parse file: " << filepath << '\n';
		return std::nullopt;
	}

	cgltf_load_buffers(&opts, data, filepath.data());

	if (data->meshes_count > 0 && data->meshes[0].primitives_count > 0) {
		cgltf_primitive* prim{ &data->meshes[0].primitives[0] };
		cgltf_accessor* accPos{ nullptr };
		cgltf_accessor* accNormal{ nullptr };

		for (cgltf_size i{}; i < prim->attributes_count; ++i) {
			if (prim->attributes[i].type == cgltf_attribute_type_position)
				accPos = prim->attributes[i].data;
			else if (prim->attributes[i].type == cgltf_attribute_type_normal)
				accNormal = prim->attributes[i].data;
		}

		if (accPos) {
			for (cgltf_size i{}; i < accPos->count; ++i) {
				std::array<float, 3> pos = { 0.0f, 0.0f, 0.0f };
				std::array<float, 3> normal = { 0.0f, 1.0f, 0.0f };

				cgltf_accessor_read_float(accPos, i, pos.data(), 3);
				if (accNormal)
					cgltf_accessor_read_float(accNormal, i, normal.data(), 3);

				meshData.vertices.push_back(pos[0]);
				meshData.vertices.push_back(pos[1]);
				meshData.vertices.push_back(pos[2]);

				meshData.vertices.push_back(normal[0]);
				meshData.vertices.push_back(normal[1]);
				meshData.vertices.push_back(normal[2]);
			}
		}

		for (cgltf_size i{}; i < prim->indices->count; ++i)
			meshData.indices.push_back(static_cast<uint32_t>(cgltf_accessor_read_index(prim->indices, i)));
	}

	cgltf_free(data);
	return meshData;
}
}
