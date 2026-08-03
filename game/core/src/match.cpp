#include "match.hpp"

#include <cmath>
#include <iostream>

#include "box3d/box3d.h"
#include "box3d/math_functions.h"
#include "components/match.hpp"
#include "components/physics.hpp"
#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"
#include "glm/ext/quaternion_trigonometric.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/trigonometric.hpp"
#include "physics.hpp"
#include "rules.hpp"

namespace core::match {

void setup(entt::registry& registry) {
	registry.ctx().emplace<Match>();
}

void reset(entt::registry& registry) {
	auto ballView = registry.view<BallTag, RigidBody, Transform>();
	for (auto [entity, tag, rb, transform] : ballView.each()) {
		b3Body_SetTransform(rb.id, physics::glmToB3(core::rules::ballSpawnPoint), b3Quat_identity);
		b3Body_SetLinearVelocity(rb.id, b3Vec3_zero);
		b3Body_SetAngularVelocity(rb.id, b3Vec3_zero);

		transform.pos = transform.prevPos = core::rules::ballSpawnPoint;
		transform.rot = transform.prevRot = glm::quat{ 1.0f, 0.0f, 0.0f, 0.0f };

		tag.hasScored = false;
	}

	int playerIndex{};
	auto playerView = registry.view<PlayerTag, RigidBody, Transform, PlayerController>();
	for (auto [entity, rb, transform, player] : playerView.each()) {
		glm::vec3 spawnPoint{ core::rules::playerSpawnPoints[playerIndex % 6] };

		glm::vec3 spawnToBall{ core::rules::ballSpawnPoint - spawnPoint };
		spawnToBall.y = 0.0f;
		spawnToBall = glm::normalize(spawnToBall);

		glm::quat rot{ glm::quatLookAt(spawnToBall, core::physics::worldUp) };
		b3Quat spawnRot{ .v = { .x = rot.x, .y = rot.y, .z = rot.z }, .s = rot.w };

		b3Body_SetTransform(rb.id, physics::glmToB3(spawnPoint), spawnRot);
		b3Body_SetLinearVelocity(rb.id, b3Vec3_zero);
		b3Body_SetAngularVelocity(rb.id, b3Vec3_zero);

		transform.pos = transform.prevPos = spawnPoint;
		transform.rot = transform.prevRot = rot;

		const glm::vec3 offsetFL{ -0.37315f, -0.310476f, -0.456852f };
		const glm::vec3 offsetFR{ 0.37315f, -0.310476f, -0.456852f };
		const glm::vec3 offsetRL{ -0.37315f, -0.310476f, 0.711378f };
		const glm::vec3 offsetRR{ 0.37315f, -0.310476f, 0.711378f };
		glm::quat rot180{ glm::angleAxis(glm::radians(180.0f), core::physics::worldUp) };

		if (player.wheelFL != entt::null) {
			auto& t = registry.get<Transform>(player.wheelFL);
			t.pos = transform.pos + (transform.rot * offsetFL);
			t.rot = transform.rot;
		}

		if (player.wheelFR != entt::null) {
			auto& t = registry.get<Transform>(player.wheelFR);
			t.pos = transform.pos + (transform.rot * offsetFR);
			t.rot = transform.rot * rot180;
		}

		if (player.wheelRL != entt::null) {
			auto& t = registry.get<Transform>(player.wheelRL);
			t.pos = transform.pos + (transform.rot * offsetRL);
			t.rot = transform.rot;
		}

		if (player.wheelRR != entt::null) {
			auto& t = registry.get<Transform>(player.wheelRR);
			t.pos = transform.pos + (transform.rot * offsetRR);
			t.rot = transform.rot * rot180;
		}

		player.up = core::physics::worldUp;
		player.steeringAngle = 0.0f;
		player.wheelAngle = 0.0f;
		player.camYaw = std::atan2(-spawnToBall.x, -spawnToBall.z);
		player.camPitch = 0.0f;
		player.camNeedSnap = true;

		++playerIndex;
	}

	auto& match{ registry.ctx().get<Match>() };
	match.scoreBlue = 0;
	match.scoreRed = 0;
	match.lastTeamToScore = -1;
	match.timer = 0.0f;

	std::cout << "MATCH RESET\n";
}

void update(entt::registry& registry, float fixedTimeStep) {
	auto& match{ registry.ctx().get<Match>() };

	switch (match.state) {
		case State::LOBBY:
			match.stateTimer = 0.0f;

			// If everyone is Ready
			reset(registry);
			match.state = State::START;
			break;
		case State::START:
			match.stateTimer += fixedTimeStep;
			if (match.stateTimer >= core::rules::startDuration) {
				match.state = State::ONGOING;
			}
			break;
		case State::ONGOING:
			match.timer += fixedTimeStep;
			if (match.timer >= core::rules::matchDuration) {
				if (match.scoreRed == match.scoreBlue) {
					std::cout << "OVERTIME\n";
					break;
				}
				if (match.scoreRed > match.scoreBlue)
					std::cout << "RED TEAM WINS!\n";
				else
					std::cout << "BLUE TEAM WINS!\n";
				match.state = State::END;
				match.stateTimer = 0.0f;
			}
			break;
		case State::END:
			match.stateTimer += fixedTimeStep;

			// If everyone pressed Rematch
			// match.stateTimer = 0.0f;
			// reset(registry);
			// match.state = State::START;

			if (match.stateTimer >= core::rules::endDuration) {
				// reset(registry);
				match.state = State::LOBBY;
			}
			break;
	}
}
}
