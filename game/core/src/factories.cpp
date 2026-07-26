#include "factories.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

#include "box3d/box3d.h"
#include "box3d/collision.h"
#include "box3d/id.h"
#include "box3d/math_functions.h"
#include "box3d/types.h"
#include "components/input.hpp"
#include "components/physics.hpp"
#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"

namespace core::factories {

void spawn_ground(b3WorldId world_id) {
	b3BodyDef body_def{ b3DefaultBodyDef() };
	body_def.position = { .x = 0.0f, .y = -2.0f, .z = 0.0f };
	b3BodyId body_id{ b3CreateBody(world_id, &body_def) };
	b3ShapeDef shape_def{ b3DefaultShapeDef() };
	shape_def.baseMaterial.friction = 0.1f;

	b3BoxHull box_hull{ b3MakeBoxHull(500.0f, 1.0f, 500.0f) };
	b3CreateHullShape(body_id, &shape_def, &box_hull.base);
}

entt::entity spawn_stadium(entt::registry& registry, b3WorldId world_id, const MeshData& meshData) {
	auto stadium_entity = registry.create();

	b3BodyDef body_def{ b3DefaultBodyDef() };
	body_def.position = b3Vec3_zero;
	body_def.type = b3_staticBody;
	b3BodyId body_id{ b3CreateBody(world_id, &body_def) };

	b3ShapeDef shape_def{ b3DefaultShapeDef() };
	shape_def.baseMaterial.friction = 0.1f;
	// shape_def.baseMaterial.restitution = 0.1f;

	std::vector<b3Vec3> b3_vertices;
	b3_vertices.reserve(meshData.vertices.size() / 6);
	for (size_t i = 0; i < meshData.vertices.size(); i += 6) {
		b3_vertices.push_back(
			{ .x = meshData.vertices[i], .y = meshData.vertices[i + 1], .z = meshData.vertices[i + 2] });
	}

	std::vector<int32_t> b3_indices;
	b3_indices.reserve(meshData.indices.size());
	for (uint16_t idx : meshData.indices) {
		b3_indices.push_back(static_cast<int32_t>(idx));
	}

	b3MeshDef mesh_def{};
	mesh_def.vertices = b3_vertices.data();
	mesh_def.vertexCount = b3_vertices.size();
	mesh_def.indices = b3_indices.data();
	mesh_def.triangleCount = b3_indices.size() / 3;
	mesh_def.weldVertices = true;
	mesh_def.identifyEdges = true;

	b3MeshData* mesh = b3CreateMesh(&mesh_def, nullptr, 0);
	b3Vec3 scale{ .x = 1.0f, .y = 1.0f, .z = 1.0f };
	b3CreateMeshShape(body_id, &shape_def, mesh, scale);

	registry.emplace<RigidBody>(stadium_entity, body_id);

	Transform t{};
	registry.emplace<Transform>(stadium_entity, t);

	return stadium_entity;
}

entt::entity spawn_ball(entt::registry& registry, b3WorldId world_id) {
	auto ball{ registry.create() };
	registry.emplace<InputComponent>(ball);

	Transform t{ .pos = { 0.0f, 1.0f, 0.0f }, .scale = { 4.0f, 4.0f, 4.0f } };
	registry.emplace<Transform>(ball, t);

	b3BodyDef body_def{ b3DefaultBodyDef() };
	body_def.type = b3_dynamicBody;
	body_def.gravityScale = 0.25f;
	body_def.angularDamping = 0.2f;
	body_def.linearDamping = 0.1f;
	body_def.position = { .x = t.pos.x, .y = t.pos.y, .z = t.pos.z };

	b3BodyId body_id{ b3CreateBody(world_id, &body_def) };
	b3ShapeDef shape_def{ b3DefaultShapeDef() };
	shape_def.density = 0.0001f;
	// shape_def.baseMaterial.rollingResistance = 0.1f;
	shape_def.baseMaterial.friction = 0.1f;
	shape_def.baseMaterial.restitution = 0.75f;

	b3Sphere sphere{ .center = b3Vec3_zero, .radius = 2.0f };
	b3CreateSphereShape(body_id, &shape_def, &sphere);

	registry.emplace<RigidBody>(ball, body_id);
	return ball;
}

entt::entity spawn_player(entt::registry& registry, b3WorldId world_id) {
	auto player{ registry.create() };
	registry.emplace<InputComponent>(player);

	Transform t{ .pos = { 0.0f, 1.0f, 10.0f } };
	registry.emplace<Transform>(player, t);

	b3BodyDef body_def{ b3DefaultBodyDef() };
	body_def.gravityScale = 0.65f;
	body_def.type = b3_dynamicBody;
	body_def.angularDamping = 0.3f;
	body_def.linearDamping = 0.6f;
	body_def.position = { .x = t.pos.x, .y = t.pos.y, .z = t.pos.z };

	b3BodyId body_id{ b3CreateBody(world_id, &body_def) };
	b3ShapeDef shape_def{ b3DefaultShapeDef() };
	shape_def.density = 10.0f;
	// shape_def.baseMaterial.rollingResistance = 0.1f;
	shape_def.baseMaterial.friction = 0.8f;
	shape_def.baseMaterial.restitution = 0.1f;

	b3Sphere sphere{ .center = b3Vec3_zero, .radius = 0.5f };
	b3CreateSphereShape(body_id, &shape_def, &sphere);

	registry.emplace<RigidBody>(player, body_id);
	return player;
}
}
