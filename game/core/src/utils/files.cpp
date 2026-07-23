#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>

namespace core::utils {
std::optional<std::string> load_file(const std::string& filepath) {
	std::ifstream file{ filepath };
	if (!file.is_open()) {
		std::cerr << "[File] Could not open file: " << filepath << '\n';
		return std::nullopt;
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}
}
