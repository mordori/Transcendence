#pragma once

#include <optional>
#include <string>

#include "components/renderable.hpp"

namespace client::systems {

MeshData create_mesh_cube();
std::optional<MeshData> load_mesh(const std::string& filepath);
Renderable create_renderable(const MeshData& mesh);
}
