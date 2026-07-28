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
#include "glm/common.hpp"
#include "glm/ext/matrix_float3x3.hpp"
#include "glm/ext/quaternion_common.hpp"
#include "glm/ext/quaternion_geometric.hpp"
#include "glm/ext/quaternion_trigonometric.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "systems/physics.hpp"

namespace core::systems {

void setupPhysics(entt::registry& registry) {
	b3WorldDef def{ b3DefaultWorldDef() };
	b3WorldId id{ b3CreateWorld(&def) };
	registry.ctx().emplace<World>(id);
}

void updatePhysics(entt::registry& registry, float dt) {
	auto worldId{ registry.ctx().get<World>().id };
	auto inputView = registry.view<InputComponent, PlayerController, RigidBody, Transform>();
	for (auto [entity, input, player, rb, transform] : inputView.each()) {
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
			player.steeringAngle = -turn * turnSpeed;
			player.steeringAngle = std::clamp(player.steeringAngle, -maxTurn, maxTurn);
		} else {
			player.steeringAngle = glm::mix(player.steeringAngle, 0.0f, 10.0f * dt);
		}

		glm::vec3 globalUp{ 0.0f, 1.0f, 0.0f };
		b3Vec3 bodyPos{ b3Body_GetPosition(rb.id) };
		glm::vec3 down{ -player.up };

		b3Vec3 rayOrigin = { //
			.x = bodyPos.x + (down.x * 0.51f),
			.y = bodyPos.y + (down.y * 0.51f),
			.z = bodyPos.z + (down.z * 0.51f)
		};
		b3Vec3 rayTranslation = { //
			.x = down.x * 0.6f,
			.y = down.y * 0.6f,
			.z = down.z * 0.6f
		};
		b3QueryFilter filter = { b3DefaultQueryFilter() };
		b3RayResult hit = { b3World_CastRayClosest(worldId, rayOrigin, rayTranslation, filter) };
		bool contact{ hit.hit };

		if (!contact) {
			b3Vec3 globalOrigin = { .x = bodyPos.x, .y = bodyPos.y, .z = bodyPos.z };
			b3Vec3 globalTranslation{ .x = 0.0f, .y = -2.0f, .z = 0.0f };
			hit = { b3World_CastRayClosest(worldId, globalOrigin, globalTranslation, filter) };
		}

		player.isGrounded = contact || hit.hit;
		glm::vec3 currentForward{ transform.rot * glm::vec3{ 0.0f, 0.0f, -1.0f } };
		glm::vec3 targetNormal = globalUp;

		if (player.isGrounded) {
			glm::vec3 surfaceNormal = glm::normalize(glm::vec3{ hit.normal.x, hit.normal.y, hit.normal.z });
			float dotSurface = glm::dot(player.up, surfaceNormal);
			if (dotSurface < 0.0f) {
				glm::vec3 rollNormal = surfaceNormal - (currentForward * glm::dot(surfaceNormal, currentForward));
				if (glm::length(rollNormal) > 0.001f)
					rollNormal = glm::normalize(rollNormal);
				else
					rollNormal = surfaceNormal;

				if (glm::dot(player.up, rollNormal) < -0.99f) {
					glm::vec3 carRight = glm::cross(currentForward, player.up);
					float nudgeDir = (glm::dot(surfaceNormal, carRight) > 0.0f) ? 1.0f : -1.0f;
					rollNormal = glm::normalize(rollNormal + carRight * 0.1f * nudgeDir);
				}
				targetNormal = rollNormal;
			} else {
				targetNormal = surfaceNormal;
			}

			if (input.jump && dotSurface >= -0.25f) {
				b3Vec3 jumpForce{ //
					.x = player.up.x * 3000.0f,
					.y = player.up.y * 3000.0f,
					.z = player.up.z * 3000.0f
				};
				b3Body_ApplyForceToCenter(rb.id, jumpForce, true);
				input.jump = false;
			}
		} else {
			if (glm::dot(player.up, globalUp) < 0.0f)
				targetNormal = -globalUp;
			else
				targetNormal = globalUp;
		}

		float alignSpeed{ (contact || player.isGrounded) ? 2.5f : 0.5f };
		if (player.isGrounded && glm::dot(player.up, targetNormal) < 0.0f)
			alignSpeed = 5.0f;

		player.up = glm::normalize(glm::mix(player.up, targetNormal, alignSpeed * dt));
		float yawDelta{ player.steeringAngle * 2.5f * dt };

		glm::quat steerRot{ glm::angleAxis(yawDelta, player.up) };
		glm::vec3 steerForward{ steerRot * currentForward };
		glm::vec3 right{ glm::cross(steerForward, player.up) };

		if (glm::length(right) < 0.001f)
			right = transform.rot * glm::vec3{ 1.0f, 0.0f, 0.0f };
		else
			right = glm::normalize(right);

		glm::vec3 trueForward{ glm::normalize(glm::cross(player.up, right)) };
		glm::mat3 matRot{ right, player.up, -trueForward };
		transform.rot = glm::quat_cast(matRot);
		b3Vec3 forward{ //
			.x = trueForward.x,
			.y = trueForward.y,
			.z = trueForward.z
		};

		move = std::max(move, -0.6f);
		float moveSpeed = (contact || player.isGrounded) ? 40.0f : 15.0f;
		if (glm::dot(player.up, targetNormal) < 0.0f && player.isGrounded)
			moveSpeed = 0.0f;
		b3Vec3 force{ //
			.x = forward.x * move * moveSpeed,
			.y = forward.y * move * moveSpeed,
			.z = forward.z * move * moveSpeed
		};

		if (force.x != 0.0f || force.y != 0.0f || force.z != 0.0f)
			b3Body_ApplyForceToCenter(rb.id, force, true);
	}

	b3World_Step(worldId, dt, 4);

	auto transformView = registry.view<Transform, RigidBody>();
	for (auto entity : transformView) {
		auto& transform = transformView.get<Transform>(entity);
		auto& rb = transformView.get<RigidBody>(entity);

		b3Vec3 pos = b3Body_GetPosition(rb.id);
		transform.pos = { pos.x, pos.y, pos.z };

		if (!registry.all_of<PlayerTag>(entity)) {
			b3Quat rot = b3Body_GetRotation(rb.id);
			transform.rot = { rot.s, rot.v.x, rot.v.y, rot.v.z };
		}
	}
}
}
