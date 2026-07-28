#pragma once

#include <optional>
#include <string>

namespace core::utils {

std::optional<std::string> loadFile(const std::string& filepath);
}
