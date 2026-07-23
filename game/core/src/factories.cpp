#include "factories.hpp"

#include "box3d/box3d.h"
#include "box3d/collision.h"
#include "box3d/id.h"
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
	b3BoxHull box_hull{ b3MakeBoxHull(500.0f, 1.0f, 500.0f) };
	b3CreateHullShape(body_id, &shape_def, &box_hull.base);
}

entt::entity spawn_player(entt::registry& registry, b3WorldId world_id) {
	auto player{ registry.create() };
	registry.emplace<InputComponent>(player);

	b3BodyDef body_def{ b3DefaultBodyDef() };
	body_def.type = b3_dynamicBody;
	body_def.position = { .x = 0.0f, .y = 1.0f, .z = 0.0f };
	b3BodyId body_id{ b3CreateBody(world_id, &body_def) };

	b3ShapeDef shape_def{ b3DefaultShapeDef() };
	shape_def.density = 1.0f;
	shape_def.baseMaterial.friction = 0.3f;
	b3BoxHull box{ b3MakeCubeHull(1.0f) };
	b3CreateHullShape(body_id, &shape_def, &box.base);

	registry.emplace<RigidBody>(player, body_id);
	return player;
}
}
