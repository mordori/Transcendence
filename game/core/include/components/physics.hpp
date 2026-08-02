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
struct BallTag {
	bool hasExploded{};
};

enum Category : uint32_t {
	COL_NONE = 0,
	COL_PLAYER = 1 << 0,
	COL_BALL = 1 << 1,
	COL_STADIUM_PLAYER = 1 << 2,
	COL_STADIUM_BALL = 1 << 3,
	COL_ALL = ~0u,
};

struct PlayerController {
	glm::vec3 up{ 0.0f, 1.0f, 0.0f };
	bool isGrounded{};
	float steeringAngle{};
	float camYaw{};
	float camPitch{ 0.2f };
	entt::entity wheelFL{ entt::null };
	entt::entity wheelFR{ entt::null };
	entt::entity wheelRL{ entt::null };
	entt::entity wheelRR{ entt::null };
	glm::vec3 velocity{};
	float wheelAngle{};
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
	std::vector<uint32_t> indices;
};
