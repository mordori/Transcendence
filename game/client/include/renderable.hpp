#pragma once

#include <optional>
#include <string>

#include "components/physics.hpp"
#include "components/renderable.hpp"

namespace client::renderer {

MeshData createMeshCube();
std::optional<MeshData> loadMesh(const std::string& filepath);
Renderable createRenderable(const MeshData& mesh);
}
