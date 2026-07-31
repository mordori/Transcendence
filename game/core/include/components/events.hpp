#pragma once

#include <vector>
#include "glm/glm.hpp"

struct ImpactEvent {
	glm::vec3	position;
	float		speed;
};

struct FrameEvents {
	std::vector<ImpactEvent> impacts; // -> registry.ctx()
};