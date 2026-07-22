#include "systems/physics.hpp"

#include <iostream>

#include "components/input.hpp"

namespace core::systems {
void update_physics(entt::registry& registry, float dt) {
	(void)dt;
	auto view = registry.view<InputComponent>();
	for (auto entity : view) {
		auto& input = view.get<InputComponent>(entity);
		if (input.up)
			std::cout << "[PHYSICS] Forward!\n";
		if (input.down)
			std::cout << "[PHYSICS] Backward!\n";
	}
}
}
