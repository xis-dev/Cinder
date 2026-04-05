// Vertex.h
#pragma once

#include "glm/vec3.hpp"
#include "glm/vec2.hpp"

struct Vertex {
    Vertex() = default;
    Vertex(glm::vec3 position, glm::vec3 normal, glm::vec2 texCoords, glm::vec3 tangent = glm::vec3(0.0f), glm::vec3 bitangent = glm::vec3(0.0f))
        : position(position), normal(normal), texCoords(texCoords), tangent(tangent), bitangent(bitangent)
    {}
    Vertex(float pos_x, float pos_y, float pos_z,
        float norm_x, float norm_y, float norm_z,
        float tex_u, float tex_v,
        float tan_x = 0.0f, float tan_y = 0.0f, float tan_z = 0.0f,
        float bitan_x = 0.0f, float bitan_y = 0.0f, float bitan_z = 0.0f)
        : position(glm::vec3(pos_x, pos_y, pos_z)),
        normal(glm::vec3(norm_x, norm_y, norm_z)),
        texCoords(glm::vec2(tex_u, tex_v)),
        tangent(glm::vec3(tan_x, tan_y, tan_z)),
        bitangent(glm::vec3(bitan_x, bitan_y, bitan_z))
    {}
    glm::vec3 position{};
    glm::vec3 normal{};
    glm::vec2 texCoords{};
    glm::vec3 tangent{};
    glm::vec3 bitangent{};
};