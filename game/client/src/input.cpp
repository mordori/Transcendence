#include "input.hpp"

#include <algorithm>
#include <cstring>

#include "components/input.hpp"
#include "components/physics.hpp"
#include "entt/core/hashed_string.hpp"
#include "entt/entity/fwd.hpp"

#ifdef __EMSCRIPTEN__
#include "emscripten/em_types.h"
#include "emscripten/html5.h"
#endif

namespace {

EM_BOOL mouseCallback(int eventType, const EmscriptenMouseEvent* e, void* userData) {
	auto* registry{ static_cast<entt::registry*>(userData) };
	auto player{ registry->ctx().get<client::input::LocalPlayerCtx>().id };
	auto& controller{ registry->get<PlayerController>(player) };

	if (eventType == EMSCRIPTEN_EVENT_MOUSEDOWN)
		emscripten_request_pointerlock("#engine-canvas", true);
	else if (eventType == EMSCRIPTEN_EVENT_MOUSEMOVE) {
		EmscriptenPointerlockChangeEvent plState{};
		emscripten_get_pointerlock_status(&plState);

		if (plState.isActive) {
			float sensitivity{ 0.00275f };
			controller.camYaw -= e->movementX * sensitivity;
			controller.camPitch -= e->movementY * sensitivity;

			controller.camPitch = std::clamp(controller.camPitch, -0.8f, 0.4f);
		}
	}
	return EM_TRUE;
}

EM_BOOL keyCallback(int eventType, const EmscriptenKeyboardEvent* e, void* userData) {
	auto* registry{ static_cast<entt::registry*>(userData) };
	auto player{ registry->ctx().get<client::input::LocalPlayerCtx>().id };
	auto& input{ registry->get<InputComponent>(player) };

	bool isDown{ (eventType == EMSCRIPTEN_EVENT_KEYDOWN) };
	bool handled{};

	switch (entt::hashed_string::value(e->code)) {
		case entt::hashed_string::value("ArrowUp"):
		case entt::hashed_string::value("KeyW"):
			input.up = isDown;
			handled = true;
			break;
		case entt::hashed_string::value("ArrowDown"):
		case entt::hashed_string::value("KeyS"):
			input.down = isDown;
			handled = true;
			break;
		case entt::hashed_string::value("ArrowLeft"):
		case entt::hashed_string::value("KeyA"):
			input.left = isDown;
			handled = true;
			break;
		case entt::hashed_string::value("ArrowRight"):
		case entt::hashed_string::value("KeyD"):
			input.right = isDown;
			handled = true;
			break;
		case entt::hashed_string::value("Space"):
			input.jump = isDown;
			handled = true;
			break;
		default: break;
	}

	return handled ? EM_TRUE : EM_FALSE;
}
}

namespace client::input {

void setup(entt::registry& registry, entt::entity player) {
	registry.ctx().emplace<LocalPlayerCtx>(player);

#ifdef __EMSCRIPTEN__
	emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, &registry, true, keyCallback);
	emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, &registry, true, keyCallback);

	emscripten_set_mousedown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, &registry, true, mouseCallback);
	emscripten_set_mousemove_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, &registry, true, mouseCallback);
#endif
}
}
