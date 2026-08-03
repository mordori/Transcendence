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
#include "glm/ext/quaternion_trigonometric.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/fwd.hpp"
#include "glm/trigonometric.hpp"
#include "rules.hpp"

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

entt::entity stadium(
	entt::registry& registry, b3WorldId worldId, const MeshData& meshData, uint32_t category, uint32_t mask) {
	auto stadium = registry.create();

	b3BodyDef bodyDef{ b3DefaultBodyDef() };
	bodyDef.position = b3Vec3_zero;
	bodyDef.type = b3_staticBody;
	b3BodyId bodyId{ b3CreateBody(worldId, &bodyDef) };

	b3ShapeDef shapeDef{ b3DefaultShapeDef() };
	shapeDef.baseMaterial.friction = 0.1f;
	shapeDef.filter.categoryBits = category;
	shapeDef.filter.maskBits = mask;

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

	Transform t{ .pos = core::rules::ballSpawnPoint, .scale = { 4.0f, 4.0f, 4.0f } };
	t.prevPos = t.pos;
	registry.emplace<Transform>(ball, t);

	b3BodyDef bodyDef{ b3DefaultBodyDef() };
	bodyDef.type = b3_dynamicBody;
	bodyDef.gravityScale = 0.35f;
	bodyDef.angularDamping = 0.5f;
	bodyDef.linearDamping = 0.3f;
	bodyDef.position = { .x = t.pos.x, .y = t.pos.y, .z = t.pos.z };

	b3BodyId bodyId{ b3CreateBody(worldId, &bodyDef) };
	b3ShapeDef shapeDef{ b3DefaultShapeDef() };
	shapeDef.filter.categoryBits = COL_BALL;
	shapeDef.filter.maskBits = COL_PLAYER | COL_STADIUM_BALL;
	shapeDef.density = 0.0001f;
	shapeDef.baseMaterial.friction = 0.1f;
	shapeDef.baseMaterial.restitution = 0.5f;
	shapeDef.enableHitEvents = true;

	b3Sphere sphere{ .center = b3Vec3_zero, .radius = 2.0f };
	b3CreateSphereShape(bodyId, &shapeDef, &sphere);

	registry.emplace<RigidBody>(ball, bodyId);
	return ball;
}

entt::entity player(entt::registry& registry, b3WorldId worldId) {
	auto player{ registry.create() };
	registry.emplace<InputComponent>(player);
	auto& controller{ registry.emplace<PlayerController>(player) };

	Transform t{ .pos = core::rules::playerSpawnPoints[0] };
	t.prevPos = t.pos;
	registry.emplace<Transform>(player, t);

	glm::vec3 offsetFR{ 0.37315f, -0.310476f, -0.456852f };
	glm::vec3 offsetFL{ -0.37315f, -0.310476f, -0.456852f };
	glm::vec3 offsetRR{ 0.37315f, -0.310476f, 0.711378f };
	glm::vec3 offsetRL{ -0.37315f, -0.310476f, 0.711378f };

	controller.wheelFR = registry.create();
	Transform tFR{ .pos = t.pos + offsetFR };
	tFR.prevPos = tFR.pos;
	glm::quat rot180 = glm::angleAxis(glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	tFR.rot = tFR.rot * rot180;
	tFR.prevRot = tFR.rot;
	registry.emplace<Transform>(controller.wheelFR, tFR);

	controller.wheelFL = registry.create();
	Transform tFL{ .pos = t.pos + offsetFL };
	tFL.prevPos = tFL.pos;
	registry.emplace<Transform>(controller.wheelFL, tFL);

	controller.wheelRR = registry.create();
	Transform tRR{ .pos = t.pos + offsetRR };
	tRR.prevPos = tRR.pos;
	tRR.rot = tRR.rot * rot180;
	tRR.prevRot = tRR.rot;
	registry.emplace<Transform>(controller.wheelRR, tRR);

	controller.wheelRL = registry.create();
	Transform tRL{ .pos = t.pos + offsetRL };
	tRL.prevPos = tRL.pos;
	registry.emplace<Transform>(controller.wheelRL, tRL);

	b3BodyDef bodyDef{ b3DefaultBodyDef() };
	bodyDef.isBullet = true;
	bodyDef.gravityScale = 1.0f;
	bodyDef.type = b3_dynamicBody;
	bodyDef.angularDamping = 2.5f;
	bodyDef.linearDamping = 0.8f;
	bodyDef.position = { .x = t.pos.x, .y = t.pos.y, .z = t.pos.z };

	b3BodyId bodyId{ b3CreateBody(worldId, &bodyDef) };
	b3ShapeDef shapeDef{ b3DefaultShapeDef() };
	shapeDef.filter.categoryBits = COL_PLAYER;
	shapeDef.filter.maskBits = COL_STADIUM_PLAYER;
	shapeDef.density = 10.0f;
	shapeDef.baseMaterial.friction = 0.0f;
	shapeDef.baseMaterial.restitution = 0.0f;

	b3Sphere sphere{ .center = b3Vec3_zero, .radius = 0.5f };
	b3CreateSphereShape(bodyId, &shapeDef, &sphere);

	b3ShapeDef shapeDefBumper{ b3DefaultShapeDef() };
	shapeDefBumper.filter.categoryBits = COL_PLAYER;
	shapeDefBumper.filter.maskBits = COL_BALL;
	shapeDefBumper.density = 0.0f;
	shapeDefBumper.baseMaterial.friction = 0.1f;
	shapeDefBumper.baseMaterial.restitution = 0.4f;

	b3Transform bumberOffset{};
	bumberOffset.p = { .x = 0.0f, .y = -0.183f, .z = 0.22f };
	bumberOffset.q = { .v = { .x = 0.0f, .y = 0.0f, .z = 0.0f }, .s = 1.0f };

	b3BoxHull bumber{ b3MakeTransformedBoxHull(0.5f, 0.75f, 1.0f, bumberOffset) };
	b3CreateHullShape(bodyId, &shapeDefBumper, &bumber.base);

	registry.emplace<RigidBody>(player, bodyId);
	return player;
}
}
