#include <iostream>
#include <utility>

#include "box3d/id.h"
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

struct GameState {
	entt::registry registry;
	bool is_running{ true };
};

void update(void* arg) {
	auto* state{ static_cast<GameState*>(arg) };
	float deltaTime{ 1.0f / 60.0f };

	core::physics::update(state->registry, deltaTime);
	client::renderer::render(state->registry);
}

int main() {
#ifndef __EMSCRIPTEN__
	std::cerr << "Error: Emscripten not found!";
	return 1;
#endif

	std::cout << "[Client] Starting...\n";
	auto* state{ new GameState() };

	core::physics::setup(state->registry);
	auto worldId{ state->registry.ctx().get<World>().id };

	core::spawn::ground(worldId);

	auto player{ core::spawn::player(state->registry, worldId) };
	state->registry.emplace<PlayerTag>(player);

	client::input::setup(state->registry, player);

	client::renderer::initWebGPU([state, player, worldId](bool success) {
		if (!success) {
			emscripten_force_exit(1);
			return;
		}

		auto meshPlayer = client::renderer::loadMesh("/models/car.glb");
		auto renderablePlayer = client::renderer::createRenderable(meshPlayer.value());
		state->registry.emplace<Renderable>(player, std::move(renderablePlayer));

		auto meshStadium = client::renderer::loadMesh("/models/cylinder.glb");
		auto stadium = core::spawn::stadium(state->registry, worldId, meshStadium.value());
		auto renderableStadium = client::renderer::createRenderable(meshStadium.value());
		state->registry.emplace<Renderable>(stadium, std::move(renderableStadium));

		auto meshBall = client::renderer::loadMesh("/models/ball.glb");
		auto ball = core::spawn::ball(state->registry, worldId);
		auto renderableBall = client::renderer::createRenderable(meshBall.value());
		state->registry.emplace<Renderable>(ball, std::move(renderableBall));

		emscripten_set_main_loop_arg(update, state, 0, false);
	});
	emscripten_exit_with_live_runtime();

	return 0;
}
