#include <glaze/glaze.hpp>
#include <glaze/json/write.hpp>
#include <iostream>
#include <string>

#include "components/physics.hpp"
#include "glaze_glm.hpp"

int main() {
	Transform pos{ //
		.position = { 1.0f, 0.0f, 42.0f },
		.rotation = { 1.0f, 1.0f, 1.0f, 1.0f }
	};
	std::string json_packet;
	[[maybe_unused]] auto ec = glz::write_json(pos, json_packet);
	std::cout << json_packet << '\n';
	return 0;
}
