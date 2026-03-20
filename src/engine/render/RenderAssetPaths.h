#pragma once

#include <string>

namespace war::RenderAssetPaths
{
    [[nodiscard]] std::string shaderBackendFolder();

    [[nodiscard]] std::string colorVertexShaderPath();
    [[nodiscard]] std::string colorFragmentShaderPath();

    [[nodiscard]] std::string textureVertexShaderPath();
    [[nodiscard]] std::string textureFragmentShaderPath();

    [[nodiscard]] std::string textureAssetPath(const std::string& fileName);
}
