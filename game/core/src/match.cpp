#include "match.hpp"

#include <cmath>

#include "box3d/box3d.h"
#include "box3d/math_functions.h"
#include "components/match.hpp"
#include "components/physics.hpp"
#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"
#include "rules.hpp"

namespace {
void checkForGoals(entt::registry& registry, Match& match) {
	auto ballView{ registry.view<BallTag, Transform>() };
	for (auto [entity, tag, transform] : ballView.each()) {
		if (transform.pos.z > core::rules::goalDistance) {
			++match.scoreBlue;
			match.lastTeamToScore = 0;
		} else if (transform.pos.z < core::rules::goalDistance) {
			++match.scoreRed;
			match.lastTeamToScore = 1;
		}
	}
}
}

namespace core::match {

void setup(entt::registry& registry) {
	registry.ctx().emplace<Match>();
}

void reset(entt::registry& registry) {
	auto ballView = registry.view<BallTag, RigidBody, Transform>();
	for (auto [entity, tag, rb, transfor] : ballView.each()) {
		b3Body_SetTransform(rb.id, core::rules::ballSpawnPoint, b3Quat_identity);
		b3Body_SetLinearVelocity(rb.id, b3Vec3_zero);
		b3Body_SetAngularVelocity(rb.id, b3Vec3_zero);
		tag.hasExploded = false;
	}

	auto playerView = registry.view<PlayerTag, RigidBody>();
	for (auto [entity, rb] : playerView.each()) {
		b3Body_SetLinearVelocity(rb.id, b3Vec3_zero);
		b3Body_SetAngularVelocity(rb.id, b3Vec3_zero);
	}
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
				match.state = State::END;
				match.stateTimer = 0.0f;
			}
			checkForGoals(registry, match);
			break;
		case State::END:
			match.stateTimer += fixedTimeStep;

			// If everyone pressed Rematch
			// match.stateTimer = 0.0f;
			// reset(registry);
			// match.state = State::START;

			if (match.stateTimer >= core::rules::endDuration) {
				reset(registry);
				match.state = State::LOBBY;
			}
			break;
	}
}
}
