#include "physics.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <random>

#include "box3d/box3d.h"
#include "box3d/id.h"
#include "box3d/math_functions.h"
#include "box3d/types.h"
#include "components/input.hpp"
#include "components/physics.hpp"
#include "entt/entity/entity.hpp"
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
#include "glm/gtc/constants.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/trigonometric.hpp"

namespace {
void updatePlayerControllers(entt::registry& registry, float fixedTimeStep, b3WorldId worldId) {
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

		move = std::max(move, -0.6f);
		glm::vec3 globalUp{ 0.0f, 1.0f, 0.0f };
		b3Vec3 bodyPos{ b3Body_GetPosition(rb.id) };
		glm::vec3 down{ -player.up };
		float mass{ b3Body_GetMass(rb.id) };
		b3Vec3 b3velocity{ b3Body_GetLinearVelocity(rb.id) };
		glm::vec3 velocity{ glm::vec3{ b3velocity.x, b3velocity.y, b3velocity.z } };
		glm::vec3 currentForward{ transform.rot * glm::vec3{ 0.0f, 0.0f, -1.0f } };

		float forwardSpeed{ glm::dot(velocity, currentForward) };
		float absSpeed{ std::abs(forwardSpeed) };
		float speedFactor{ std::clamp(absSpeed / 40.0f, 0.0f, 1.0f) };

		float maxTurn{ glm::mix(0.7f, 0.25f, speedFactor) };
		float targetSteering{ -turn * maxTurn };
		float steerSpeed{ (turn != 0.0f) ? 2.5f : 4.7f };
		player.steeringAngle = glm::mix(player.steeringAngle, targetSteering, steerSpeed * fixedTimeStep);

		b3Vec3 rayOrigin = { .x = bodyPos.x, .y = bodyPos.y, .z = bodyPos.z };
		float rayLen = 0.75f;
		b3Vec3 rayTranslation = { .x = down.x * rayLen, .y = down.y * rayLen, .z = down.z * rayLen };
		b3QueryFilter filter = { b3DefaultQueryFilter() };
		filter.categoryBits = COL_PLAYER;
		filter.maskBits = COL_STADIUM_PLAYER;
		b3RayResult hit = { b3World_CastRayClosest(worldId, rayOrigin, rayTranslation, filter) };

		if (!hit.hit) {
			b3Vec3 globalOrigin = { .x = bodyPos.x, .y = bodyPos.y, .z = bodyPos.z };
			b3Vec3 globalTranslation{ .x = 0.0f, .y = -0.7f, .z = 0.0f };
			hit = { b3World_CastRayClosest(worldId, globalOrigin, globalTranslation, filter) };
		}

		player.isGrounded = hit.hit;
		glm::vec3 targetNormal = globalUp;
		glm::vec3 surfaceNormal{};
		if (player.isGrounded) {
			surfaceNormal = glm::normalize(glm::vec3{ hit.normal.x, hit.normal.y, hit.normal.z });
			float dotSurface = glm::dot(player.up, surfaceNormal);
			if (dotSurface < 0.0f) {
				if (dotSurface < 0.9f)
					player.steeringAngle = 0.0f;
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
				float jumpSpeed{ 16.0f * mass };
				b3Vec3 jumpImpulse{ //
					.x = player.up.x * jumpSpeed,
					.y = player.up.y * jumpSpeed,
					.z = player.up.z * jumpSpeed
				};
				b3Body_ApplyLinearImpulseToCenter(rb.id, jumpImpulse, true);
				input.jump = false;
			}
		} else {
			targetNormal = (glm::dot(player.up, globalUp) < 0.0f) ? -globalUp : globalUp;
			if (velocity.y < -0.2f) {
				float extraGravity{ 15.0f * mass };
				b3Vec3 fallForce{ //
					.x = 0.0f,
					.y = -extraGravity,
					.z = 0.0f
				};
				b3Body_ApplyForceToCenter(rb.id, fallForce, true);
			}
		}

		float alignSpeed{ player.isGrounded ? 8.5f : 2.5f };

		player.up = glm::normalize(glm::mix(player.up, targetNormal, alignSpeed * fixedTimeStep));

		float yawDelta{};
		if (player.isGrounded) {
			float turnSensitivity{ 1.5f };
			float rotationSpeed{ std::clamp(forwardSpeed, -12.0f, 15.0f) };
			yawDelta = (rotationSpeed / turnSensitivity) * std::sin(player.steeringAngle) * fixedTimeStep;
		} else {
			float airSpinSpeed{ 4.0f };
			float dir{ (move < 0.0f) ? -1.0f : 1.0f };
			yawDelta = (player.steeringAngle / maxTurn) * dir * airSpinSpeed * fixedTimeStep;
		}

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
		b3Vec3 b3forward{ //
			.x = trueForward.x,
			.y = trueForward.y,
			.z = trueForward.z
		};

		float moveSpeed{};
		moveSpeed = (glm::dot(player.up, targetNormal) < 0.8f) ? 20.0f : 30.0f;
		if (!player.isGrounded)
			moveSpeed = std::min(moveSpeed, 20.0f);
		if (move < 0.0f)
			moveSpeed *= 0.8f;
		moveSpeed = move * moveSpeed * mass;
		b3Vec3 moveForce{ //
			.x = b3forward.x * moveSpeed,
			.y = b3forward.y * moveSpeed,
			.z = b3forward.z * moveSpeed
		};

		if (moveForce.x != 0.0f || moveForce.y != 0.0f || moveForce.z != 0.0f)
			b3Body_ApplyForceToCenter(rb.id, moveForce, true);

		glm::vec3 forward{ b3forward.x, b3forward.y, b3forward.z };

		if (player.isGrounded) {
			float rightSpeed{ glm::dot(velocity, right) };

			float grip{ rightSpeed * 0.85f * mass };

			b3Vec3 lateralImpulse{ //
				.x = -right.x * grip,
				.y = -right.y * grip,
				.z = -right.z * grip
			};
			b3Body_ApplyLinearImpulseToCenter(rb.id, lateralImpulse, true);

			float dir{ (forwardSpeed >= 0.0f) ? 1.0f : -1.0f };
			float drag{ std::abs(rightSpeed) * 0.35f * mass * dir };
			b3Vec3 scrubImpulse{ //
				.x = -forward.x * drag,
				.y = -forward.y * drag,
				.z = -forward.z * drag
			};
			b3Body_ApplyLinearImpulseToCenter(rb.id, scrubImpulse, true);

			// if (glm::dot(player.up, globalUp) > -0.5f) {
			// 	float downPull{ std::abs(forwardSpeed * 0.0f * mass) };
			// 	b3Vec3 downForce{ //
			// 		.x = -surfaceNormal.x * downPull,
			// 		.y = -surfaceNormal.y * downPull,
			// 		.z = -surfaceNormal.z * downPull
			// 	};
			// 	b3Body_ApplyForceToCenter(rb.id, downForce, true);
			// }
		}

		player.velocity = core::physics::b3ToGlm(b3Body_GetLinearVelocity(rb.id));
	}
}

void updateBall(entt::registry& registry) {
	auto inputView = registry.view<BallTag, RigidBody, Transform>();
	for (auto [entity, tag, rb, transform] : inputView.each()) {
		float mass{ b3Body_GetMass(rb.id) };
		b3Vec3 b3velocity{ b3Body_GetLinearVelocity(rb.id) };
		glm::vec3 velocity{ glm::vec3{ b3velocity.x, b3velocity.y, b3velocity.z } };

		if (velocity.y < -0.1f) {
			float extraGravity{ 5.0f * mass };
			b3Vec3 fallForce{ //
				.x = 0.0f,
				.y = -extraGravity,
				.z = 0.0f
			};
			b3Body_ApplyForceToCenter(rb.id, fallForce, true);
		}

		static std::mt19937 mt{ std::random_device{}() };
		std::uniform_real_distribution<float> randomX{ -0.07f, 0.07f };
		float goalDistance{ 54.5f };

		if (transform.pos.z > goalDistance || transform.pos.z < -goalDistance) {
			auto playerView = registry.view<PlayerTag, RigidBody, Transform>();
			for (auto [entity, rbPlayer, tPlayer] : playerView.each()) {
				glm::vec3 ballToPlayer{ tPlayer.pos - transform.pos };
				ballToPlayer.y = 0.0f;
				float distance{ glm::length(ballToPlayer) };

				if (distance < 75.0f && distance > 0.001f) {
					b3Body_SetLinearVelocity(rbPlayer.id, b3Vec3_zero);

					glm::vec3 dir{ glm::normalize(ballToPlayer) };
					dir.y =
						1.5f * std::clamp(1.0f - std::clamp(((tPlayer.pos.y - 0.5f) / 10.0f), 0.0f, 1.0f), 0.0f, 1.0f);
					dir.z += (glm::sign(dir.z) * glm::abs(tPlayer.pos.x) / 40.0f * 5.0f) + (glm::sign(dir.z) * 0.5f);
					dir.z = std::min(dir.z, dir.y);
					dir = glm::normalize(dir);
					b3Vec3 goalImpulse{ //
						.x = dir.x,
						.y = dir.y,
						.z = dir.z
					};
					goalImpulse *= ((std::clamp(1.0f - (distance / 75.0f), 0.0f, 1.0f) * 0.6f) + 0.4f) * 50.0f *
						b3Body_GetMass(rbPlayer.id);
					b3Body_ApplyLinearImpulseToCenter(rbPlayer.id, goalImpulse, true);
				}
			}
			b3Body_SetLinearVelocity(rb.id, b3Vec3_zero);
			tag.hasExploded = true;
		}
		if (transform.pos.z > goalDistance) {
			b3Vec3 goalImpulse{ //
				.x = randomX(mt),
				.y = 0.07f,
				.z = -0.09f
			};
			b3Body_ApplyLinearImpulseToCenter(rb.id, goalImpulse, true);
		} else if (transform.pos.z < -goalDistance) {
			b3Vec3 goalImpulse{ //
				.x = randomX(mt),
				.y = 0.07f,
				.z = 0.09f
			};
			b3Body_ApplyLinearImpulseToCenter(rb.id, goalImpulse, true);
		}

		if (transform.pos.z < 50.0f && transform.pos.z > -50.0f)
			tag.hasExploded = false;
	}
}

void updateTransforms(entt::registry& registry) {
	auto transformView = registry.view<Transform, RigidBody>();
	for (auto entity : transformView) {
		auto& transform = transformView.get<Transform>(entity);
		auto& rb = transformView.get<RigidBody>(entity);

		b3Vec3 pos = b3Body_GetPosition(rb.id);
		transform.pos = { pos.x, pos.y, pos.z };

		if (!registry.all_of<PlayerTag>(entity)) {
			b3Quat rot = b3Body_GetRotation(rb.id);
			transform.rot = glm::quat{ rot.s, rot.v.x, rot.v.y, rot.v.z };
		} else {
			auto& player{ registry.get<PlayerController>(entity) };

			glm::vec3 offsetFL{ -0.37315f, -0.310476f, -0.456852f };
			glm::vec3 offsetFR{ 0.37315f, -0.310476f, -0.456852f };
			glm::vec3 offsetRL{ -0.37315f, -0.310476f, 0.711378f };
			glm::vec3 offsetRR{ 0.37315f, -0.310476f, 0.711378f };

			glm::vec3 posDelta{ transform.pos - transform.prevPos };
			glm::vec3 currentForward{ transform.rot * glm::vec3{ 0.0f, 0.0f, -1.0f } };
			float distanceForward{ glm::dot(posDelta, currentForward) };

			float tireRadius{ 2.0f };
			float maxVisualDist{ tireRadius * 0.4f };
			float visualDist = std::clamp(distanceForward, -maxVisualDist, maxVisualDist);
			player.wheelAngle -= (visualDist / tireRadius);
			player.wheelAngle = std::fmod(player.wheelAngle, glm::two_pi<float>());

			glm::quat rotSteer{ glm::angleAxis(player.steeringAngle, glm::vec3{ 0.0f, 1.0f, 0.0f }) };

			if (player.wheelFL != entt::null) {
				glm::quat rotRoll{ glm::angleAxis(player.wheelAngle, glm::vec3{ 1.0f, 0.0f, 0.0f }) };
				auto& wheelT = registry.get<Transform>(player.wheelFL);
				wheelT.pos = transform.pos + (transform.rot * offsetFL);
				wheelT.rot = transform.rot * rotSteer * rotRoll;
			}

			if (player.wheelFR != entt::null) {
				glm::quat rotRoll{ glm::angleAxis(-player.wheelAngle, glm::vec3{ 1.0f, 0.0f, 0.0f }) };
				auto& wheelT = registry.get<Transform>(player.wheelFR);
				wheelT.pos = transform.pos + (transform.rot * offsetFR);
				glm::quat rot180 = glm::angleAxis(glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				wheelT.rot = transform.rot * rotSteer * rot180 * rotRoll;
			}

			if (player.wheelRL != entt::null) {
				glm::quat rotRoll{ glm::angleAxis(player.wheelAngle, glm::vec3{ 1.0f, 0.0f, 0.0f }) };
				auto& wheelT = registry.get<Transform>(player.wheelRL);
				wheelT.pos = transform.pos + (transform.rot * offsetRL);
				wheelT.rot = transform.rot * rotRoll;
			}

			if (player.wheelRR != entt::null) {
				glm::quat rotRoll{ glm::angleAxis(-player.wheelAngle, glm::vec3{ 1.0f, 0.0f, 0.0f }) };
				auto& wheelT = registry.get<Transform>(player.wheelRR);
				wheelT.pos = transform.pos + (transform.rot * offsetRR);
				glm::quat rot180 = glm::angleAxis(glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				wheelT.rot = transform.rot * rot180 * rotRoll;
			}
		}
	}
}

void setPreviousTransforms(entt::registry& registry) {
	auto allTransforms = registry.view<Transform>();
	for (auto entity : allTransforms) {
		auto& t = allTransforms.get<Transform>(entity);
		t.prevPos = t.pos;
		t.prevRot = t.rot;
	}
}
}

namespace core::physics {

void setup(entt::registry& registry) {
	b3WorldDef def{ b3DefaultWorldDef() };
	def.gravity = { .x = 0.0f, .y = -25.0f, .z = 0.0f };
	b3WorldId id{ b3CreateWorld(&def) };
	registry.ctx().emplace<World>(id);
}

void update(entt::registry& registry, float fixedTimeStep) {
	auto worldId{ registry.ctx().get<World>().id };

	setPreviousTransforms(registry);
	updatePlayerControllers(registry, fixedTimeStep, worldId);
	updateBall(registry);
	b3World_Step(worldId, fixedTimeStep, 4);
	updateTransforms(registry);
}
}
