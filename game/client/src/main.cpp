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

#include "box3d/box3d.h"
#include "box3d/id.h"
#include "box3d/types.h"
#include "components/input.hpp"
#include "components/physics.hpp"
#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"
#include "factories.hpp"
#include "systems/input.hpp"
#include "systems/physics.hpp"
#include "systems/render.hpp"

#ifdef __EMSCRIPTEN__
#include "emscripten.h"
#include "emscripten/emscripten.h"
#include "emscripten/html5.h"
#endif

struct GameState {
	entt::registry registry;
	bool is_running{ true };
};

void main_loop(void* arg) {
	auto* state{ static_cast<GameState*>(arg) };
	float dt{ 1.0f / 60.0f };

	core::systems::update_physics(state->registry, dt);
	client::systems::render(state->registry);
}

int main() {
	std::cout << "--- Standalone Player Starting ---\n";
	auto* state{ new GameState() };

	core::systems::setup_physics(state->registry);
	auto world_id{ state->registry.ctx().get<World>().id };

	core::factories::spawn_ground(world_id);
	auto local_player{ core::factories::spawn_player(state->registry, world_id) };

	client::systems::setup_input(state->registry, local_player);
#ifdef __EMSCRIPTEN__
	client::systems::init_webgpu([state](bool success) {
		if (!success) {
			emscripten_force_exit(1);
			return;
		}
		emscripten_set_main_loop_arg(main_loop, state, 0, false);
	});
	emscripten_exit_with_live_runtime();
#else
	std::cerr << "Error: Emscripten not found!";
	return 1;
#endif
	return 0;
}
