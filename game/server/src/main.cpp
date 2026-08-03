#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>

#include "components/physics.hpp"
#include "entt/entity/fwd.hpp"
#include "match.hpp"
#include "physics.hpp"
#include "spawn.hpp"

void tick(entt::registry& registry) {
	static auto lastTime{ std::chrono::steady_clock::now() };
	auto currentTime{ std::chrono::steady_clock::now() };

	float deltaTime{ std::chrono::duration<float>(currentTime - lastTime).count() };
	deltaTime = std::min(deltaTime, 0.1f);

	lastTime = currentTime;
	static float timeAccumulator{};
	timeAccumulator += deltaTime;

	// TODO: Lets see if we can run this 120 Hz later, but broadcast as 60 Hz
	constexpr float fixedTimeStep = 1.0f / 60.0f;
	auto tickStart{ std::chrono::steady_clock::now() };

	while (timeAccumulator >= fixedTimeStep) {
		core::physics::update(registry, fixedTimeStep);
		core::match::update(registry, fixedTimeStep);
		timeAccumulator -= fixedTimeStep;
	}

	// TODO: Listen and process incoming connections
	// TODO: Serialize and broadcast state to clients

	auto tickEnd{ std::chrono::steady_clock::now() };
	float tickTime{ std::chrono::duration<float>(tickEnd - tickStart).count() };
	float sleepTime{ fixedTimeStep - timeAccumulator - tickTime };
	if (sleepTime > 0.0f)
		std::this_thread::sleep_for(std::chrono::duration<float>(fixedTimeStep - tickTime));
}

int main() {
	std::cout << "[SERVER] Starting...";
	entt::registry registry;

	core::physics::setup(registry);
	core::match::setup(registry);

	// TODO: Signal handler for shutdown
	bool isRunning{ true };
	while (isRunning)
		tick(registry);

	return 0;
}
