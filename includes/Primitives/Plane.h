// Plane.h
#pragma once

#include "Graphics/Vertex.h"
#include <vector>

namespace Plane
{
    inline std::vector<Vertex> vertices
    {
        Vertex{-0.5f, 0.0f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f},
        Vertex{ 0.5f, 0.0f,  0.5f,  0.0f, 1.0f, 0.0f, 1.0f, 1.0f},
        Vertex{-0.5f, 0.0f, -0.5f,  0.0f, 1.0f, 0.0f, 0.0f, 0.0f},
        Vertex{ 0.5f, 0.0f, -0.5f,  0.0f, 1.0f, 0.0f, 1.0f, 0.0f}
    };

    inline std::vector<unsigned int> indices
    {
        0, 1, 2,
        1, 3, 2
    };

    inline void computeTangents()
    {
        // Triangle 1 (0,1,2)
        glm::vec3 edge1 = vertices[1].position - vertices[0].position;
        glm::vec3 edge2 = vertices[2].position - vertices[0].position;
        glm::vec2 deltaUV1 = vertices[1].texCoords - vertices[0].texCoords;
        glm::vec2 deltaUV2 = vertices[2].texCoords - vertices[0].texCoords;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
        glm::vec3 tangent1, bitangent1;
        tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

        // Triangle 2 (1,3,2)
        edge1 = vertices[3].position - vertices[1].position;
        edge2 = vertices[2].position - vertices[1].position;
        deltaUV1 = vertices[3].texCoords - vertices[1].texCoords;
        deltaUV2 = vertices[2].texCoords - vertices[1].texCoords;

        f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
        glm::vec3 tangent2, bitangent2;
        tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

        // Average per vertex
        vertices[0].tangent = tangent1;
        vertices[0].bitangent = bitangent1;
        vertices[1].tangent = (tangent1 + tangent2) * 0.5f;
        vertices[1].bitangent = (bitangent1 + bitangent2) * 0.5f;
        vertices[2].tangent = (tangent1 + tangent2) * 0.5f;
        vertices[2].bitangent = (bitangent1 + bitangent2) * 0.5f;
        vertices[3].tangent = tangent2;
        vertices[3].bitangent = bitangent2;
    }
}