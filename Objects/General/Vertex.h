#pragma once

#include <Math/Vec3.h>

struct Vertex
{
    Vertex() = default;
    Vertex(Vec3f position, Vec3f normal, Vec2f texCoords): position(position), normal(normal), texCoords(texCoords){}
    Vertex(float pos_x, float pos_y, float pos_z,
           float norm_x, float norm_y, float norm_z,
           float tex_u, float tex_v):
                                        position(Vec3f(pos_x, pos_y, pos_z)),
                                        normal(Vec3f(norm_x, norm_y, norm_z)),
                                        texCoords(Vec2f(tex_u, tex_v)){}
    Vec3f position{};
    Vec3f normal{};
    Vec2f texCoords{};
};