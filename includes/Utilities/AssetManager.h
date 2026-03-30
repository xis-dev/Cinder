#pragma once

#include "ResourceManager.h"
struct AssetManager
{
    ResourceManager<Shader> shaders{};
    ResourceManager<Texture> textures{};
    ResourceManager<Material> materials{};
    ResourceManager<Model> models{};
};