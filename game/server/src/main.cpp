#include <chrono>
#include <iostream>
#include <thread>

#include "components/physics.hpp"
#include "entt/entity/fwd.hpp"
#include "physics.hpp"
#include "spawn.hpp"

int main() {
	std::cout << "[SERVER] Starting...";
	entt::registry registry;

	core::physics::setup(registry);
	auto worldID = registry.ctx().get<World>().id;

	core::spawn::ground(worldID);

	const float deltaTime{ 1.0f / 60.0f };
	const auto tickDuration{ std::chrono::duration<float>(deltaTime) };

	bool isRunning{ true };
	while (isRunning) {
		auto tickStart{ std::chrono::steady_clock::now() };

		// TODO: Listen and process incoming connections

		core::physics::update(registry, deltaTime);

		// TODO: Serialize and broadcast state to clients

		auto tickEnd{ std::chrono::steady_clock::now() - tickStart };
		if (tickEnd < tickDuration)
			std::this_thread::sleep_for(tickDuration - tickEnd);
	}

	return 0;
}
