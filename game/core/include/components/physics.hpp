#pragma once

#include <cstdint>
#include <vector>

#include "box3d/id.h"
#include "entt/entity/entity.hpp"
#include "entt/entity/fwd.hpp"
#include "glm/fwd.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

struct PlayerTag {};
struct BallTag {};

struct PlayerController {
	glm::vec3 up{ 0.0f, 1.0f, 0.0f };
	bool isGrounded{};
	float steeringAngle{};
	float camYaw{};
	float camPitch{ 0.2f };
	entt::entity frontLeftWheel{ entt::null };
	entt::entity frontRightWheel{ entt::null };
};

struct Transform {
	glm::vec3 pos{};
	glm::quat rot{ 1.0f, 0.0f, 0.0f, 0.0f };
	glm::vec3 scale{ 1.0f };
	glm::vec3 prevPos{};
	glm::quat prevRot{ 1.0f, 0.0f, 0.0f, 0.0f };
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
