#include "components/renderable.hpp"

#include <array>
#include <cstdint>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"
#include "components/physics.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "render.hpp"
#include "renderable.hpp"
#include "webgpu/webgpu_cpp.h"

namespace client::renderer {

Renderable createRenderable(const MeshData& mesh) {
	Renderable r{};
	auto& ctx = getRenderContext();

	wgpu::BufferDescriptor ubo{};
	ubo.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
	ubo.size = sizeof(glm::mat4);
	r.uniformBuffer = ctx.device.CreateBuffer(&ubo);

	wgpu::BindGroupEntry bgEntry{};
	bgEntry.binding = 0;
	bgEntry.buffer = r.uniformBuffer;
	bgEntry.size = sizeof(glm::mat4);

	wgpu::BindGroupDescriptor bgDesc{};
	bgDesc.layout = ctx.bindGroupLayout;
	bgDesc.entryCount = 1;
	bgDesc.entries = &bgEntry;
	r.bindGroup = ctx.device.CreateBindGroup(&bgDesc);

	r.indexCount = mesh.indices.size();

	wgpu::BufferDescriptor vbo{};
	vbo.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
	vbo.size = sizeof(float) * mesh.vertices.size();
	r.vertexBuffer = ctx.device.CreateBuffer(&vbo);
	ctx.queue.WriteBuffer(r.vertexBuffer, 0, mesh.vertices.data(), vbo.size);

	wgpu::BufferDescriptor ibo{};
	ibo.usage = wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst;
	ibo.size = sizeof(uint32_t) * mesh.indices.size();
	r.indexBuffer = ctx.device.CreateBuffer(&ibo);
	ctx.queue.WriteBuffer(r.indexBuffer, 0, mesh.indices.data(), ibo.size);

	return r;
}

MeshData createMeshCube() {
	MeshData mesh{};

	// clang-format off
	mesh.vertices = {
		-1, -1,  1,   1, 0, 0,
		 1, -1,  1,   0, 1, 0,
		 1,  1,  1,   0, 0, 1,
		-1,  1,  1,   1, 1, 0,
		-1, -1, -1,   1, 0, 1,
		 1, -1, -1,   0, 1, 1,
		 1,  1, -1,   1, 1, 1,
		-1,  1, -1,   0, 0, 0
	};

	mesh.indices = {
		0,1,2, 2,3,0, 1,5,6, 6,2,1, 5,4,7, 7,6,5,
		4,0,3, 3,7,4, 3,2,6, 6,7,3, 4,5,1, 1,0,4
	};
	// clang-format on

	return mesh;
}

std::optional<MeshData> loadMesh(const std::string& filepath) {
	MeshData mesh{};
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

				mesh.vertices.push_back(pos[0]);
				mesh.vertices.push_back(pos[1]);
				mesh.vertices.push_back(pos[2]);

				mesh.vertices.push_back((normal[0] + 1.0f) * 0.5f);
				mesh.vertices.push_back((normal[1] + 1.0f) * 0.5f);
				mesh.vertices.push_back((normal[2] + 1.0f) * 0.5f);
			}
		}

		for (cgltf_size i{}; i < prim->indices->count; ++i)
			mesh.indices.push_back(static_cast<uint32_t>(cgltf_accessor_read_index(prim->indices, i)));
	}

	cgltf_free(data);

	return mesh;
}
}
