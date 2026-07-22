#include "systems/input.hpp"

#include <cstring>

#include "components/input.hpp"
#include "entt/entity/fwd.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>
#endif

namespace client::systems {
struct LocalPlayerCtx {
	entt::entity id;
};

#ifdef __EMSCRIPTEN__

EM_BOOL key_callback(int eventType, const EmscriptenKeyboardEvent* e, void* userData) {
	auto* registry{ static_cast<entt::registry*>(userData) };

	auto player{ registry->ctx().get<LocalPlayerCtx>().id };
	auto& input{ registry->get<InputComponent>(player) };

	bool is_down{ (eventType == EMSCRIPTEN_EVENT_KEYDOWN) };
	bool handled{};

	switch (entt::hashed_string::value(e->code)) {
		case entt::hashed_string::value("ArrowUp"):
		case entt::hashed_string::value("KeyW"):
			input.up = is_down;
			handled = true;
			break;
		case entt::hashed_string::value("ArrowDown"):
		case entt::hashed_string::value("KeyS"):
			input.down = is_down;
			handled = true;
			break;
		case entt::hashed_string::value("ArrowLeft"):
		case entt::hashed_string::value("KeyA"):
			input.left = is_down;
			handled = true;
			break;
		case entt::hashed_string::value("ArrowRight"):
		case entt::hashed_string::value("KeyD"):
			input.right = is_down;
			handled = true;
			break;
		case entt::hashed_string::value("Space"): handled = true; break;
		default: break;
	}

	return handled ? EM_TRUE : EM_FALSE;
}
#endif

void setup_input(entt::registry& registry, entt::entity local_player) {
	registry.ctx().emplace<LocalPlayerCtx>(local_player);

#ifdef __EMSCRIPTEN__
	emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, &registry, true, key_callback);
	emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, &registry, true, key_callback);
#endif
}

}
