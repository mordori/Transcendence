#include "network.hpp"

#include <cstdint>
#include <glaze/core/refl.hpp>
#include <glaze/glaze.hpp>
#include <glaze/json/write.hpp>
#include <string>
#include <vector>

#include "components/network.hpp"
#include "components/physics.hpp"
#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"

namespace core::network {

std::string buildSnapshot(entt::registry& registry, uint32_t currentTick) {
	Snapshot snapshot{};
	snapshot.tick = currentTick;

	auto playerView = registry.view<PlayerTag, Transform>();
	for (auto [entity, transform] : playerView.each()) {
		snapshot.players.push_back({ //
			.id = static_cast<uint32_t>(entity),
			.pos = transform.pos,
			.rot = transform.rot });
	}

	auto ballView = registry.view<BallTag, Transform>();
	for (auto [entity, transform] : ballView.each()) {
		snapshot.balls.push_back({ //
			.id = static_cast<uint32_t>(entity),
			.pos = transform.pos,
			.rot = transform.rot });
	}

	auto payload = glz::write_json(snapshot);
	if (!payload) {
		return "";
	}
	return payload.value();
}
}
