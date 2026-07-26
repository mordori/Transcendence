#pragma once

#include <optional>
#include <string>

namespace core::utils {

std::optional<std::string> load_file(const std::string& filepath);
}
