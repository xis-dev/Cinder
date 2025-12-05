#include <vector>
#include <Objects/General/Vertex.h>

namespace Cube {
    inline std::vector<Vertex> vertices = {
        // positions          // normals           // texture coords
        // front
        Vertex(-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f),
        Vertex( 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f),
        Vertex( 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f),
        Vertex(-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f),
        // back
        Vertex(-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f),
        Vertex( 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f),
        Vertex( 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f),
        Vertex(-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f),
        // left
        Vertex(-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f),
        Vertex(-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f),
        Vertex(-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f),
        Vertex(-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f),
        // right
        Vertex( 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f),
        Vertex( 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f),
        Vertex( 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f),
        Vertex( 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f),
        // top
        Vertex(-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f),
        Vertex( 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f),
        Vertex( 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f),
        Vertex(-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f),
        // bottom
        Vertex(-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f),
        Vertex( 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f),
        Vertex( 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f),
        Vertex(-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f)
    };

    inline std::vector<unsigned int> indices = {
        // front
        0, 1, 2,
        2, 3, 0,
        // back
        4, 5, 6,
        6, 7, 4,
        // left
        8, 9, 10,
        10, 11, 8,
        // right
        12, 13, 14,
        14, 15, 12,
        // top
        16, 17, 18,
        18, 19, 16,
        // bottom
        20, 21, 22,
        22, 23, 20
    };
}