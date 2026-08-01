#include <algorithm>
#include <chrono>
#include <iostream>
#include <utility>

#include "audio.hpp"
#include "box3d/id.h"
#include "components/audio.hpp"
#include "components/physics.hpp"
#include "components/renderable.hpp"
#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"
#include "input.hpp"
#include "physics.hpp"
#include "render.hpp"
#include "renderable.hpp"
#include "spawn.hpp"

#ifdef __EMSCRIPTEN__
#include "emscripten.h"
#include "emscripten/emscripten.h"
#include "emscripten/html5.h"
#endif

struct ClientState {
	entt::registry registry;
	// Network receiver
	// WebSocket

	bool is_running{ true };
};

// ClientState* clientState{ nullptr };

// Stand-alone player version
void update(void* arg) {
	auto* state{ static_cast<ClientState*>(arg) };
	static auto lastTime{ std::chrono::steady_clock::now() };
	auto currentTime{ std::chrono::steady_clock::now() };

	float deltaTime{ std::chrono::duration<float>(currentTime - lastTime).count() };
	// std::cout << "deltaTime: " << deltaTime << " (FPS: " << 1.0f / deltaTime << ")\n";
	deltaTime = std::min(deltaTime, 0.1f);
	static float smoothDelta = deltaTime;
	smoothDelta = (smoothDelta * 0.9f) + (deltaTime * 0.1f);

	lastTime = currentTime;

	static float timeAccumulator{};
	timeAccumulator += smoothDelta;

	const float fixedTimeStep = 1.0f / 60.0f;

	while (timeAccumulator >= fixedTimeStep) {
		core::physics::update(state->registry, fixedTimeStep);
		timeAccumulator -= fixedTimeStep;
	}

	float alpha = timeAccumulator / fixedTimeStep;
	alpha = std::clamp(alpha, 0.0f, 1.0f);
	client::audio::update(state->registry, smoothDelta);
	client::renderer::render(state->registry, smoothDelta, alpha);
}

// TODO: Dedicated server version
void update() {
	// 1. Process incoming snapshots from server
	// 2. Poll local player inputs
	// 3. Send inputs to server

	// client::renderer::render(clientState->registry);
}

// int main() {
// 	std::cout << "[Client] Starting...\n";
// 	clientState = new ClientState();

// 	client::input::setup(clientState->registry, player);

// 	client::renderer::initWebGPU([](bool success) {
// 		if (!success) {
// 			std::cerr << "[WebGPU] Initialization failed. Aborting.\n";
// 			emscripten_force_exit(1);
// 			return;
// 		}

// 		emscripten_set_main_loop(update, 0, false);
// 	});
// 	emscripten_exit_with_live_runtime();

// 	return 0;
// }

int main() {
	std::cout << "[Client] Starting...\n";
	auto* clientState = new ClientState();

	core::physics::setup(clientState->registry);
	client::audio::setup(clientState->registry);
	auto worldId{ clientState->registry.ctx().get<World>().id };

	// core::spawn::ground(worldId);

	auto player{ core::spawn::player(clientState->registry, worldId) };
	clientState->registry.emplace<PlayerTag>(player);

	client::input::setup(clientState->registry, player);

	client::renderer::initWebGPU([clientState, player, worldId](bool success) {
		if (!success) {
			emscripten_force_exit(1);
			return;
		}

		auto meshPlayer = client::renderer::loadMesh("/models/car.glb");
		auto renderablePlayer = client::renderer::createRenderable(meshPlayer.value());
		clientState->registry.emplace<Renderable>(player, std::move(renderablePlayer));

		auto& controller{ clientState->registry.get<PlayerController>(player) };
		auto meshWheel = client::renderer::loadMesh("/models/wheel.glb");
		auto renderableFLWheel = client::renderer::createRenderable(meshWheel.value());
		auto renderableFRWheel = client::renderer::createRenderable(meshWheel.value());
		auto renderableRRWheel = client::renderer::createRenderable(meshWheel.value());
		auto renderableRLWheel = client::renderer::createRenderable(meshWheel.value());
		clientState->registry.emplace<Renderable>(controller.wheelFL, std::move(renderableFLWheel));
		clientState->registry.emplace<Renderable>(controller.wheelFR, std::move(renderableFRWheel));
		clientState->registry.emplace<Renderable>(controller.wheelRL, std::move(renderableRRWheel));
		clientState->registry.emplace<Renderable>(controller.wheelRR, std::move(renderableRLWheel));

		auto meshStadium = client::renderer::loadMesh("/models/cylinder.glb");
		auto stadium = core::spawn::stadium(clientState->registry, worldId, meshStadium.value());
		auto renderableStadium = client::renderer::createRenderable(meshStadium.value());
		clientState->registry.emplace<Renderable>(stadium, std::move(renderableStadium));

		auto meshBall = client::renderer::loadMesh("/models/ball.glb");
		auto ball = core::spawn::ball(clientState->registry, worldId);
		auto renderableBall = client::renderer::createRenderable(meshBall.value());
		clientState->registry.emplace<Renderable>(ball, std::move(renderableBall));

		emscripten_set_main_loop_arg(update, clientState, 0, false);
	});
	emscripten_exit_with_live_runtime();

	return 0;
}
