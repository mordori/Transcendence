#include "systems/physics.hpp"

#include <cmath>
#include <iostream>

#include "box3d/box3d.h"
#include "box3d/id.h"
#include "box3d/math_functions.h"
#include "box3d/types.h"
#include "components/input.hpp"
#include "components/physics.hpp"
#include "entt/entity/fwd.hpp"

namespace core::systems {
void setup_physics(entt::registry& registry) {
	b3WorldDef def{ b3DefaultWorldDef() };
	b3WorldId id{ b3CreateWorld(&def) };
	registry.ctx().emplace<World>(id);
}

void update_physics(entt::registry& registry, float dt) {
	auto world_id{ registry.ctx().get<World>().id };
	auto input_view = registry.view<InputComponent, RigidBody>();
	for (auto entity : input_view) {
		auto& input = input_view.get<InputComponent>(entity);
		auto& rb = input_view.get<RigidBody>(entity);

		float move_x = 0.0f;
		float move_z = 0.0f;

		if (input.up)
			move_z -= 1.0f;
		if (input.down)
			move_z += 1.0f;
		if (input.left)
			move_x -= 1.0f;
		if (input.right)
			move_x += 1.0f;

		if (move_x != 0.0f && move_z != 0.0f) {
			float length = std::sqrt((move_x * move_x) + (move_z * move_z));
			move_x /= length;
			move_z /= length;
		}

		float speed{ 5.0f };
		b3Vec3 force{ .x = move_x * speed, .y = 0.0f, .z = move_z * speed };

		if (force.x != 0.0f || force.z != 0.0f)
			b3Body_ApplyForceToCenter(rb.id, force, true);
	}
	b3World_Step(world_id, dt, 4);

	auto transform_view = registry.view<Transform, RigidBody>();
	for (auto entity : transform_view) {
		auto& transform = transform_view.get<Transform>(entity);
		auto& rb = transform_view.get<RigidBody>(entity);

		b3Vec3 pos = b3Body_GetPosition(rb.id);
		b3Quat rot = b3Body_GetRotation(rb.id);

		transform.pos = { pos.x, pos.y, pos.z };
		transform.rot = { rot.s, rot.v.x, rot.v.y, rot.v.z };
	}
}
}
