#include "systems/input.hpp"

#include <algorithm>
#include <cstring>

#include "components/input.hpp"
#include "entt/core/hashed_string.hpp"
#include "entt/entity/fwd.hpp"

#ifdef __EMSCRIPTEN__
#include "emscripten/em_types.h"
#include "emscripten/html5.h"
#endif

namespace client::systems {

struct LocalPlayerCtx {
	entt::entity id;
};

EM_BOOL mouse_callback(int eventType, const EmscriptenMouseEvent* e, void* userData) {
	auto* registry{ static_cast<entt::registry*>(userData) };
	auto player{ registry->ctx().get<LocalPlayerCtx>().id };
	auto& input{ registry->get<InputComponent>(player) };

	if (eventType == EMSCRIPTEN_EVENT_MOUSEDOWN)
		emscripten_request_pointerlock("#engine-canvas", true);
	else if (eventType == EMSCRIPTEN_EVENT_MOUSEMOVE) {
		EmscriptenPointerlockChangeEvent pl_state{};
		emscripten_get_pointerlock_status(&pl_state);

		if (pl_state.isActive) {
			float sensitivity{ 0.00275f };
			input.cam_yaw -= e->movementX * sensitivity;
			input.cam_pitch -= e->movementY * sensitivity;

			input.cam_pitch = std::clamp(input.cam_pitch, -1.5f, 1.5f);
		}
	}
	return EM_TRUE;
}

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
		case entt::hashed_string::value("Space"):
			input.jump = is_down;
			handled = true;
			break;
		default: break;
	}

	return handled ? EM_TRUE : EM_FALSE;
}

void setup_input(entt::registry& registry, entt::entity local_player) {
	registry.ctx().emplace<LocalPlayerCtx>(local_player);

#ifdef __EMSCRIPTEN__
	emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, &registry, true, key_callback);
	emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, &registry, true, key_callback);

	emscripten_set_mousedown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, &registry, true, mouse_callback);
	emscripten_set_mousemove_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, &registry, true, mouse_callback);
#endif
}

}
