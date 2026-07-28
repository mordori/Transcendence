// #include <glaze/glaze.hpp>
// #include <glaze/json/write.hpp>
// #include <iostream>
// #include <string>

// #include "components/physics.hpp"
// #include "glaze_glm.hpp"

// int main() {
// 	Transform pos{ //
// 		.position = { 1.0f, 0.0f, 42.0f },
// 		.rotation = { 1.0f, 1.0f, 1.0f, 1.0f }
// 	};
// 	std::string json_packet;
// 	[[maybe_unused]] auto ec = glz::write_json(pos, json_packet);
// 	std::cout << json_packet << '\n';
// 	return 0;
// }

#include <iostream>
#include <utility>

#include "box3d/id.h"
#include "components/physics.hpp"
#include "components/renderable.hpp"
#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"
#include "factories.hpp"
#include "systems/input.hpp"
#include "systems/physics.hpp"
#include "systems/render.hpp"
#include "systems/renderable.hpp"

#ifdef __EMSCRIPTEN__
#include "emscripten.h"
#include "emscripten/emscripten.h"
#include "emscripten/html5.h"
#endif

struct GameState {
	entt::registry registry;
	bool is_running{ true };
};

void mainLoop(void* arg) {
	auto* state{ static_cast<GameState*>(arg) };
	float dt{ 1.0f / 60.0f };

	core::systems::updatePhysics(state->registry, dt);
	client::systems::render(state->registry);
}

int main() {
#ifndef __EMSCRIPTEN__
	std::cerr << "Error: Emscripten not found!";
	return 1;
#endif

	std::cout << "--- Standalone Player Starting ---\n";
	auto* state{ new GameState() };

	core::systems::setupPhysics(state->registry);
	auto worldId{ state->registry.ctx().get<World>().id };

	core::factories::spawnGround(worldId);

	auto player{ core::factories::spawnPlayer(state->registry, worldId) };
	state->registry.emplace<PlayerTag>(player);

	client::systems::setupInput(state->registry, player);

	client::systems::initWebGPU([state, player, worldId](bool success) {
		if (!success) {
			emscripten_force_exit(1);
			return;
		}

		auto meshPlayer = client::systems::loadMesh("/models/car.glb");
		auto renderablePlayer = client::systems::createRenderable(meshPlayer.value());
		state->registry.emplace<Renderable>(player, std::move(renderablePlayer));

		auto meshStadium = client::systems::loadMesh("/models/cylinder.glb");
		auto stadium = core::factories::spawnStadium(state->registry, worldId, meshStadium.value());
		auto renderableStadium = client::systems::createRenderable(meshStadium.value());
		state->registry.emplace<Renderable>(stadium, std::move(renderableStadium));

		auto meshBall = client::systems::loadMesh("/models/ball.glb");
		auto ball = core::factories::spawnBall(state->registry, worldId);
		auto renderableBall = client::systems::createRenderable(meshBall.value());
		state->registry.emplace<Renderable>(ball, std::move(renderableBall));

		emscripten_set_main_loop_arg(mainLoop, state, 0, false);
	});
	emscripten_exit_with_live_runtime();

	return 0;
}
