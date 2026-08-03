#include <algorithm>
#include <chrono>
#include <iostream>
#include <utility>

#include "audio.hpp"
#include "box3d/id.h"
#include "components/audio.hpp"
#include "components/physics.hpp"
#include "components/renderer.hpp"
#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"
#include "input.hpp"
#include "match.hpp"
#include "physics.hpp"
#include "renderer.hpp"
#include "spawn.hpp"

#ifdef __EMSCRIPTEN__
#include "emscripten.h"
#include "emscripten/emscripten.h"
#include "emscripten/html5.h"
#endif

struct Client {
	entt::registry registry;
	// Network receiver
	// WebSocket

	bool isRunning{ true };
};

// Client* _client{ nullptr };

// Stand-alone player version
void update(void* arg) {
	auto* state{ static_cast<Client*>(arg) };
	static float smoothDelta{};
	static float timeAccumulator{};
	static auto prevTickStart{ std::chrono::steady_clock::now() };

	auto tickStart{ std::chrono::steady_clock::now() };
	float deltaTime{ std::chrono::duration<float>(tickStart - prevTickStart).count() };
	deltaTime = std::min(deltaTime, 0.1f);
	smoothDelta = (smoothDelta * 0.9f) + (deltaTime * 0.1f);

	prevTickStart = tickStart;
	timeAccumulator += smoothDelta;

	constexpr float fixedTimeStep = 1.0f / 60.0f;

	while (timeAccumulator >= fixedTimeStep) {
		core::physics::update(state->registry, fixedTimeStep);
		core::match::update(state->registry, fixedTimeStep);
		timeAccumulator -= fixedTimeStep;
	}

	float alpha = std::clamp(timeAccumulator / fixedTimeStep, 0.0f, 1.0f);
	client::audio::update(state->registry, smoothDelta);
	client::renderer::render(state->registry, smoothDelta, alpha);
}

// TODO: Dedicated server version
// void update() {
// 	// 1. Process incoming snapshots from server
// 	// 2. Poll local player inputs
// 	// 3. Send inputs to server

// 	static auto lastTime{ std::chrono::steady_clock::now() };
// 	auto currentTime{ std::chrono::steady_clock::now() };

// 	float deltaTime{ std::chrono::duration<float>(currentTime - lastTime).count() };
// 	deltaTime = std::min(deltaTime, 0.1f);
// 	static float smoothDelta = deltaTime;
// 	smoothDelta = (smoothDelta * 0.9f) + (deltaTime * 0.1f);

// 	lastTime = currentTime;

// 	static float timeAccumulator{};
// 	timeAccumulator += smoothDelta;

// 	const float fixedTimeStep = 1.0f / 60.0f;

// 	while (timeAccumulator >= fixedTimeStep)
// 		timeAccumulator -= fixedTimeStep;

// 	float alpha = timeAccumulator / fixedTimeStep;
// 	alpha = std::clamp(alpha, 0.0f, 1.0f);
// 	client::audio::update(_client->registry, smoothDelta);
// 	client::renderer::render(_client->registry, smoothDelta, alpha);
// }

int main() {
	std::cout << "[Client] Starting...\n";
	auto* _client = new Client();
	// _client = new Client();

	core::physics::setup(_client->registry);
	client::audio::setup(_client->registry);
	auto worldId{ _client->registry.ctx().get<World>().id };

	auto player{ core::spawn::player(_client->registry, worldId) };
	_client->registry.emplace<PlayerTag>(player);
	_client->registry.emplace<Camera>(player);
	client::audio::attachEngine(_client->registry, player);

	client::input::setup(_client->registry, player);
	core::match::setup(_client->registry);

	// client::renderer::setup([player, worldId](bool success) {
	client::renderer::setup([_client, player, worldId](bool success) {
		if (!success) {
			std::cerr << "[WebGPU] Initialization failed.\n";
			emscripten_force_exit(1);
			return;
		}

		auto meshPlayer = client::renderer::loadMesh("/models/car.glb");
		auto renderablePlayer = client::renderer::createRenderable(meshPlayer.value());
		_client->registry.emplace<Renderable>(player, std::move(renderablePlayer));

		auto& controller{ _client->registry.get<PlayerController>(player) };
		auto meshWheel = client::renderer::loadMesh("/models/wheel.glb");
		auto renderableFLWheel = client::renderer::createRenderable(meshWheel.value());
		auto renderableFRWheel = client::renderer::createRenderable(meshWheel.value());
		auto renderableRRWheel = client::renderer::createRenderable(meshWheel.value());
		auto renderableRLWheel = client::renderer::createRenderable(meshWheel.value());
		_client->registry.emplace<Renderable>(controller.wheelFL, std::move(renderableFLWheel));
		_client->registry.emplace<Renderable>(controller.wheelFR, std::move(renderableFRWheel));
		_client->registry.emplace<Renderable>(controller.wheelRL, std::move(renderableRRWheel));
		_client->registry.emplace<Renderable>(controller.wheelRR, std::move(renderableRLWheel));

		auto meshStadiumCol = client::renderer::loadMesh("/models/stadium_col.glb");
		core::spawn::stadium(_client->registry, worldId, meshStadiumCol.value(), COL_STADIUM_PLAYER, COL_PLAYER);

		auto meshStadium = client::renderer::loadMesh("/models/stadium.glb");
		auto stadiumBall =
			core::spawn::stadium(_client->registry, worldId, meshStadium.value(), COL_STADIUM_BALL, COL_BALL);
		auto renderableStadium = client::renderer::createRenderable(meshStadium.value());
		_client->registry.emplace<Renderable>(stadiumBall, std::move(renderableStadium));

		auto meshBall = client::renderer::loadMesh("/models/ball.glb");
		auto ball = core::spawn::ball(_client->registry, worldId);
		auto renderableBall = client::renderer::createRenderable(meshBall.value());
		_client->registry.emplace<Renderable>(ball, std::move(renderableBall));

		emscripten_set_main_loop_arg(update, _client, 0, false);
		// emscripten_set_main_loop(update, 0, false);
	});
	emscripten_exit_with_live_runtime();

	return 0;
}
