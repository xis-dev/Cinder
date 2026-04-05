// Cube.h
#pragma once

#include <vector>
#include <Graphics/Vertex.h>

namespace Cube
{
    inline std::vector<Vertex> vertices = {
        // front
        Vertex(-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f),
        Vertex(0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f),
        Vertex(0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f),
        Vertex(-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f),
        // back
        Vertex(-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f),
        Vertex(0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f),
        Vertex(0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f),
        Vertex(-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f),
        // left
        Vertex(-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f),
        Vertex(-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f),
        Vertex(-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f),
        Vertex(-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f),
        // right
        Vertex(0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f),
        Vertex(0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f),
        Vertex(0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f),
        Vertex(0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f),
        // top
        Vertex(-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f),
        Vertex(0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f),
        Vertex(0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f),
        Vertex(-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f),
        // bottom
        Vertex(-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f),
        Vertex(0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f),
        Vertex(0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f),
        Vertex(-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f)
    };

    inline std::vector<unsigned int> indices = {
        0,1,2, 2,3,0,
        4,5,6, 6,7,4,
        8,9,10, 10,11,8,
        12,13,14, 14,15,12,
        16,17,18, 18,19,16,
        20,21,22, 22,23,20
    };

    inline void computeTangents()
    {
        for (int face = 0; face < 6; ++face)
        {
            int idx0 = face * 4;
            int idx1 = idx0 + 1;
            int idx2 = idx0 + 2;
            int idx3 = idx0 + 3;

            // Triangle 1 (idx0, idx1, idx2)
            
            glm::vec3 edge1 = vertices[idx1].position - vertices[idx0].position;
            glm::vec3 edge2 = vertices[idx2].position - vertices[idx0].position;
            glm::vec2 deltaUV1 = vertices[idx1].texCoords - vertices[idx0].texCoords;
            glm::vec2 deltaUV2 = vertices[idx2].texCoords - vertices[idx0].texCoords;

            float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
            glm::vec3 tangent1, bitangent1;
            tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
            tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
            tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
            bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
            bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
            bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

            // Triangle 2 (idx1, idx3, idx2)
            edge1 = vertices[idx3].position - vertices[idx1].position;
            edge2 = vertices[idx2].position - vertices[idx1].position;
            deltaUV1 = vertices[idx3].texCoords - vertices[idx1].texCoords;
            deltaUV2 = vertices[idx2].texCoords - vertices[idx1].texCoords;

            f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
            glm::vec3 tangent2, bitangent2;
            tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
            tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
            tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
            bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
            bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
            bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

            // Average per vertex
            vertices[idx0].tangent = tangent1;
            vertices[idx0].bitangent = bitangent1;
            vertices[idx1].tangent = (tangent1 + tangent2) * 0.5f;
            vertices[idx1].bitangent = (bitangent1 + bitangent2) * 0.5f;
            vertices[idx2].tangent = (tangent1 + tangent2) * 0.5f;
            vertices[idx2].bitangent = (bitangent1 + bitangent2) * 0.5f;
            vertices[idx3].tangent = tangent2;
            vertices[idx3].bitangent = bitangent2;
        }
    }
}