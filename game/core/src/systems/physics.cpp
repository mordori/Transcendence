#include "systems/physics.hpp"

#include <algorithm>
#include <cmath>

#include "box3d/box3d.h"
#include "box3d/id.h"
#include "box3d/math_functions.h"
#include "box3d/types.h"
#include "components/input.hpp"
#include "components/physics.hpp"
#include "entt/entity/fwd.hpp"
#include "glm/ext/quaternion_trigonometric.hpp"
#include "glm/ext/vector_float3.hpp"

namespace core::systems {

void setup_physics(entt::registry& registry) {
	b3WorldDef def{ b3DefaultWorldDef() };
	b3WorldId id{ b3CreateWorld(&def) };
	registry.ctx().emplace<World>(id);
}

void update_physics(entt::registry& registry, float dt) {
	auto world_id{ registry.ctx().get<World>().id };
	auto input_view = registry.view<InputComponent, RigidBody, Transform>();
	for (auto [entity, input, rb, transform] : input_view.each()) {
		float move = 0.0f;
		float turn = 0.0f;

		if (input.up)
			move += 1.0f;
		if (input.down)
			move -= 1.0f;
		if (input.left)
			turn -= 1.0f;
		if (input.right)
			turn += 1.0f;

		float turnSpeed{ 0.5f };
		float maxTurn{ 0.5f };

		if (turn != 0.0f) {
			input.steering_angle = -turn * turnSpeed;
			input.steering_angle = std::clamp(input.steering_angle, -maxTurn, maxTurn);
		} else {
			input.steering_angle = std::lerp(input.steering_angle, 0.0f, 10.0f * dt);
		}

		float turnSharpness{ 2.5f };
		input.yaw += input.steering_angle * turnSharpness * dt;
		transform.rot = glm::angleAxis(input.yaw, glm::vec3(0.0f, 1.0f, 0.0f));

		glm::vec3 fwd_glm = transform.rot * glm::vec3(0.0f, 0.0f, -1.0f);
		b3Vec3 forward{ //
			.x = fwd_glm.x,
			.y = fwd_glm.y,
			.z = fwd_glm.z
		};

		move = std::max(move, -0.6f);
		float moveSpeed{ 40.0f };
		b3Vec3 force{ //
			.x = forward.x * move * moveSpeed,
			.y = 0.0f,
			.z = forward.z * move * moveSpeed
		};
		if (force.x != 0.0f || force.z != 0.0f)
			b3Body_ApplyForceToCenter(rb.id, force, true);

		b3Vec3 body_pos{ b3Body_GetPosition(rb.id) };
		if (input.jump && body_pos.y < 1.0f) {
			b3Vec3 jumpForce{ .x = 0.0f, .y = 3000.0f, .z = 0.0f };
			b3Body_ApplyForceToCenter(rb.id, jumpForce, true);
			input.jump = false;
		}
	}
	b3World_Step(world_id, dt, 4);

	auto transform_view = registry.view<Transform, RigidBody>();
	for (auto entity : transform_view) {
		auto& transform = transform_view.get<Transform>(entity);
		auto& rb = transform_view.get<RigidBody>(entity);

		b3Vec3 pos = b3Body_GetPosition(rb.id);
		transform.pos = { pos.x, pos.y, pos.z };

		if (!registry.all_of<PlayerTag>(entity)) {
			b3Quat rot = b3Body_GetRotation(rb.id);
			transform.rot = { rot.s, rot.v.x, rot.v.y, rot.v.z };
		}
	}
}
}
