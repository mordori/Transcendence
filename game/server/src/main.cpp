#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>

#include "components/physics.hpp"
#include "entt/entity/fwd.hpp"
#include "match.hpp"
#include "physics.hpp"
#include "spawn.hpp"
#include "network.hpp"

void tick(entt::registry& registry) {
	static float timeAccumulator{};
	static auto prevTickStart{ std::chrono::steady_clock::now() };

	auto tickStart{ std::chrono::steady_clock::now() };
	float deltaTime{ std::chrono::duration<float>(tickStart - prevTickStart).count() };
	deltaTime = std::min(deltaTime, 0.1f);

	prevTickStart = tickStart;
	timeAccumulator += deltaTime;

	// TODO: Lets see if we can run this 120 Hz later, but broadcast as 60 Hz
	constexpr float fixedTimeStep = 1.0f / 60.0f;

	while (timeAccumulator >= fixedTimeStep) {
		core::physics::update(registry, fixedTimeStep);
		core::match::update(registry, fixedTimeStep);
		timeAccumulator -= fixedTimeStep;
	}

	// Serialize current state to JSON and broadcast to all clients
	// fill in serialize() - this is the "plug" point.
	std::string snapshot = core::match::serialize(registry);
	network::broadcast(snapshot);

	auto tickEnd{ std::chrono::steady_clock::now() };
	float tickTime{ std::chrono::duration<float>(tickEnd - tickStart).count() };
	float sleepTime{ fixedTimeStep - timeAccumulator - tickTime };
	if (sleepTime > 0.0f)
		std::this_thread::sleep_for(std::chrono::duration<float>(sleepTime));
}

int main() {
	std::cout << "[SERVER] Starting...";
	entt::registry registry;

	core::physics::setup(registry);
	core::match::setup(registry);

	network::init(9001);

	// TODO: Signal handler for shutdown
	bool isRunning{ true };
	while (isRunning)
		tick(registry);

	return 0;
}
