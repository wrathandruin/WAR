#pragma once

#include <string>

namespace war::RenderAssetPaths
{
    [[nodiscard]] std::string shaderBackendFolder();
    [[nodiscard]] std::string colorVertexShaderPath();
    [[nodiscard]] std::string colorFragmentShaderPath();
}
