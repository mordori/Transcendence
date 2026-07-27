#pragma once

#include <cstdint>
#include <vector>

#include "box3d/id.h"
#include "glm/fwd.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

struct PlayerTag {};

struct PlayerComponent {
	bool is_grounded{};
};

struct Transform {
	glm::vec3 pos{};
	glm::quat rot{ 1.0f, 0.0f, 0.0f, 0.0f };
	glm::vec3 scale{ 1.0f };
};

struct World {
	b3WorldId id{};
};

struct RigidBody {
	b3BodyId id{};
};

struct MeshData {
	std::vector<float> vertices;
	std::vector<uint16_t> indices;
};
