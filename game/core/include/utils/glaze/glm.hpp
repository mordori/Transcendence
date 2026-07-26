#pragma once

#include "glaze/core/common.hpp"
#include "glaze/core/meta.hpp"
#include "glaze/glaze.hpp"
#include "glm/fwd.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

template <>
struct glz::meta<glm::vec2> {
	using T = glm::vec2;
	static constexpr auto value = glz::array(&T::x, &T::y);
};

template <>
struct glz::meta<glm::vec3> {
	using T = glm::vec3;
	static constexpr auto value = glz::array(&T::x, &T::y, &T::z);
};

template <>
struct glz::meta<glm::quat> {
	using T = glm::quat;
	static constexpr auto value = glz::array(&T::w, &T::x, &T::y, &T::z);
};
