#pragma once

#include "ResourceManager.h"
#include "Shader.h"
#include "Material.h"
#include "Model.h"

struct AssetManager
{
    ResourceManager<Shader> shaders{};
    ResourceManager<Texture> textures{};
    ResourceManager<Material> materials{};
    ResourceManager<Model> models{};

};
