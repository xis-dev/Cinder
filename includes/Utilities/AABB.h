#pragma once
#include <vector>

#include "geometric.hpp"
#include "vec3.hpp"

struct AABB
{
    AABB(const glm::vec3& min, const glm::vec3& max): boundsMin(min), boundsMax(max)
    {
        const glm::vec3 minToMax = boundsMax - boundsMin;
        center = boundsMin + 0.5f * minToMax;
        diagonalDistance = glm::length(minToMax);
    }
    glm::vec3 boundsMin;
    glm::vec3 boundsMax;
    glm::vec3 center{};
    float diagonalDistance;

    static AABB getAABB(const std::vector<glm::vec3>& positions)
    {
        glm::vec3 minBounds{std::numeric_limits<float>::max()};
        glm::vec3 maxBounds{std::numeric_limits<float>::min()};
        for (auto& p: positions)
        {
            if (p.x < minBounds.x) minBounds.x = p.x;
            if (p.y < minBounds.y) minBounds.y = p.y;
            if (p.z < minBounds.z) minBounds.z = p.z;

            if (p.x > maxBounds.x) maxBounds.x = p.x;
            if (p.y > maxBounds.y) maxBounds.y = p.y;
            if (p.z > maxBounds.z) maxBounds.z = p.z;
        }

        return AABB{minBounds, maxBounds};
    }
};
