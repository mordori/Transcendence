#include "spawn.hpp"

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
#include "glm/ext/vector_float3.hpp"

namespace core::spawn {

void ground(b3WorldId worldId) {
	b3BodyDef bodyDef{ b3DefaultBodyDef() };
	bodyDef.position = { .x = 0.0f, .y = -2.0f, .z = 0.0f };
	b3BodyId bodyId{ b3CreateBody(worldId, &bodyDef) };
	b3ShapeDef shapeDef{ b3DefaultShapeDef() };
	shapeDef.baseMaterial.friction = 0.1f;

	b3BoxHull boxHull{ b3MakeBoxHull(500.0f, 1.0f, 500.0f) };
	b3CreateHullShape(bodyId, &shapeDef, &boxHull.base);
}

entt::entity stadium(entt::registry& registry, b3WorldId worldId, const MeshData& meshData) {
	auto stadium = registry.create();

	b3BodyDef bodyDef{ b3DefaultBodyDef() };
	bodyDef.position = b3Vec3_zero;
	bodyDef.type = b3_staticBody;
	b3BodyId bodyId{ b3CreateBody(worldId, &bodyDef) };

	b3ShapeDef shapeDef{ b3DefaultShapeDef() };
	shapeDef.baseMaterial.friction = 0.1f;

	std::vector<b3Vec3> b3Vertices;
	b3Vertices.reserve(meshData.vertices.size() / 6);
	for (size_t i = 0; i < meshData.vertices.size(); i += 6) {
		b3Vertices.push_back(
			{ .x = meshData.vertices[i], .y = meshData.vertices[i + 1], .z = meshData.vertices[i + 2] });
	}

	std::vector<int32_t> b3Indices;
	b3Indices.reserve(meshData.indices.size());
	for (uint16_t idx : meshData.indices) {
		b3Indices.push_back(static_cast<int32_t>(idx));
	}

	b3MeshDef meshDef{};
	meshDef.vertices = b3Vertices.data();
	meshDef.vertexCount = b3Vertices.size();
	meshDef.indices = b3Indices.data();
	meshDef.triangleCount = b3Indices.size() / 3;
	meshDef.weldVertices = true;
	meshDef.identifyEdges = true;

	b3MeshData* mesh = b3CreateMesh(&meshDef, nullptr, 0);
	b3Vec3 scale{ .x = 1.0f, .y = 1.0f, .z = 1.0f };
	b3CreateMeshShape(bodyId, &shapeDef, mesh, scale);

	registry.emplace<RigidBody>(stadium, bodyId);

	Transform t{};
	registry.emplace<Transform>(stadium, t);

	return stadium;
}

entt::entity ball(entt::registry& registry, b3WorldId worldId) {
	auto ball{ registry.create() };
	registry.emplace<BallTag>(ball);

	Transform t{ .pos = { 0.0f, 1.0f, 0.0f }, .scale = { 4.0f, 4.0f, 4.0f } };
	t.prevPos = t.pos;
	registry.emplace<Transform>(ball, t);

	b3BodyDef bodyDef{ b3DefaultBodyDef() };
	bodyDef.type = b3_dynamicBody;
	bodyDef.gravityScale = 0.25f;
	bodyDef.angularDamping = 0.5f;
	bodyDef.linearDamping = 0.3f;
	bodyDef.position = { .x = t.pos.x, .y = t.pos.y, .z = t.pos.z };

	b3BodyId bodyId{ b3CreateBody(worldId, &bodyDef) };
	b3ShapeDef shapeDef{ b3DefaultShapeDef() };
	shapeDef.density = 0.0001f;
	shapeDef.baseMaterial.friction = 0.1f;
	shapeDef.baseMaterial.restitution = 0.5f;

	b3Sphere sphere{ .center = b3Vec3_zero, .radius = 2.0f };
	b3CreateSphereShape(bodyId, &shapeDef, &sphere);

	registry.emplace<RigidBody>(ball, bodyId);
	return ball;
}

entt::entity player(entt::registry& registry, b3WorldId worldId) {
	auto player{ registry.create() };
	registry.emplace<InputComponent>(player);
	auto& controller{ registry.emplace<PlayerController>(player) };

	Transform t{ .pos = { 0.0f, 1.0f, 10.0f } };
	t.prevPos = t.pos;
	registry.emplace<Transform>(player, t);

	controller.frontRightWheel = registry.create();
	Transform tFR{};
	registry.emplace<Transform>(controller.frontRightWheel, tFR);

	controller.frontLeftWheel = registry.create();
	Transform tFL{};
	// tFL.scale.x *= -1.0f;
	registry.emplace<Transform>(controller.frontLeftWheel, tFL);

	b3BodyDef bodyDef{ b3DefaultBodyDef() };
	bodyDef.isBullet = true;
	bodyDef.gravityScale = 1.0f;
	bodyDef.type = b3_dynamicBody;
	bodyDef.angularDamping = 2.5f;
	bodyDef.linearDamping = 0.8f;
	bodyDef.position = { .x = t.pos.x, .y = t.pos.y, .z = t.pos.z };

	b3BodyId bodyId{ b3CreateBody(worldId, &bodyDef) };
	b3ShapeDef shapeDef{ b3DefaultShapeDef() };
	shapeDef.density = 10.0f;
	shapeDef.baseMaterial.friction = 0.0f;
	shapeDef.baseMaterial.restitution = 0.0f;

	b3Sphere sphere{ .center = b3Vec3_zero, .radius = 0.5f };
	b3CreateSphereShape(bodyId, &shapeDef, &sphere);

	registry.emplace<RigidBody>(player, bodyId);
	return player;
}
}
