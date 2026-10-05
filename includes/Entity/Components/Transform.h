#pragma once

#include "Component.h"

#include "vec3.hpp"
#include "mat4x4.hpp"

class Entity;

struct Transform: Component {

    glm::vec3 m_position{};
    glm::vec3 m_scale{1.0f};
    // TODO: Change to Quaternions(Display in Euler Angles)
    glm::vec3 m_rotationAxis{0.0f, 0.0f, 1.0f};
    float m_rotationAngle{};

    static glm::vec3 getWorldPosition(const Entity& e);

    static glm::mat4 getGlobalTransform(const Entity& e);

    static glm::mat4 getTransformMatrix(const Transform& t);
};


