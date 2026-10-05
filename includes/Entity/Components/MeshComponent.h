#pragma once

#include "Component.h"

#include "glm/vec3.hpp"

class Model;

// Mesh component for every entity that may be displayed by a 3d mesh
struct MeshComponent : Component {
    MeshComponent(Model* model): m_model(model){}
    Model* m_model{nullptr};
};

