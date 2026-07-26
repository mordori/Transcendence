#include "factories.hpp"

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

entt::entity spawn_player(entt::registry& registry, b3WorldId world_id) {
	auto player{ registry.create() };
	registry.emplace<InputComponent>(player);

	Transform t{ .pos = { 0.0f, 1.0f, 0.0f }, .rot{ 1.0f, 0.0f, 0.0f, 0.0f } };
	registry.emplace<Transform>(player, t);

	b3BodyDef body_def{ b3DefaultBodyDef() };
	body_def.type = b3_dynamicBody;
	body_def.angularDamping = 0.3f;
	body_def.linearDamping = 0.7f;
	body_def.position = { .x = t.pos.x, .y = t.pos.y, .z = t.pos.z };

	b3BodyId body_id{ b3CreateBody(world_id, &body_def) };

	b3ShapeDef shape_def{ b3DefaultShapeDef() };
	shape_def.density = 1.0f;
	// shape_def.baseMaterial.rollingResistance = 0.1f;
	shape_def.baseMaterial.friction = 0.8f;
	b3Sphere sphere{ .center = b3Vec3_zero, .radius = 0.5f };
	b3CreateSphereShape(body_id, &shape_def, &sphere);

	registry.emplace<RigidBody>(player, body_id);
	return player;
}
}
