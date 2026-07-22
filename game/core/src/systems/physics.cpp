#include "systems/physics.hpp"

#include <iostream>

#include "box3d/box3d.h"
#include "box3d/id.h"
#include "box3d/math_functions.h"
#include "box3d/types.h"
#include "components/input.hpp"
#include "components/physics.hpp"
#include "entt/entity/fwd.hpp"

namespace core::systems {
void setup_physics(entt::registry& registry) {
	b3WorldDef def{ b3DefaultWorldDef() };
	b3WorldId id{ b3CreateWorld(&def) };
	registry.ctx().emplace<World>(id);
}

void update_physics(entt::registry& registry, float dt) {
	auto world_id{ registry.ctx().get<World>().id };
	auto view = registry.view<InputComponent, RigidBody>();
	for (auto entity : view) {
		auto& input = view.get<InputComponent>(entity);
		auto& rb = view.get<RigidBody>(entity);

		b3Vec3 force{};
		float speed{ 50.0f };

		if (input.up)
			force.z -= speed;
		if (input.down)
			force.z += speed;
		if (input.left)
			force.x -= speed;
		if (input.right)
			force.x += speed;

		if (force.x != 0.0f || force.z != 0.0f)
			b3Body_ApplyForceToCenter(rb.id, force, true);
	}
	b3World_Step(world_id, dt, 4);

	static int tick = 0;
	if (tick++ % 60 == 0) {
		for (auto entity : view) {
			auto& rb = view.get<RigidBody>(entity);
			b3Vec3 pos = b3Body_GetPosition(rb.id);
			std::cout << "Player posX: " << pos.x << ", posY: " << pos.y << ", posZ: " << pos.z << '\n';
		}
	}
}
}
