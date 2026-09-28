 //
// Created by PC on 29-Jul-26.
//

#ifndef FOLDER_DIRECTIONALSHADOWPASS_H
#define FOLDER_DIRECTIONALSHADOWPASS_H
#include <string>
#include <unordered_map>

#include "Camera.h"
#include "RenderPass.h"

#include "glm/vec2.hpp"

struct AABB
{
    AABB(const glm::vec3& min, const glm::vec3& max): boundsMin(min), boundsMax(max)
    {
        glm::vec3 minToMax = boundsMax - boundsMin;
        center = boundsMin + (0.5f * minToMax);
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

 class Scene;

 class DirectionalShadowPass: public IRenderPassConfigurer<const Camera&, Scene*>
{

 public:
     float correctionStrength{0.5f};
     std::vector<float> m_cascadeSplits;

     glm::vec2 mainResolution{1024, 1024};
     unsigned texArray{};

     Shader* m_shader{nullptr};

     std::vector<std::string> cascadeUniformStrings;

     void updateSplits(std::vector<float> &splits, float near, float far, float linearCorrection);

 public:
     FrameBuffer m_buffer;

     int numberOfSplits{2};

     glm::mat4 lightTransform{1.0f};

     DirectionalShadowPass() = default;
     DirectionalShadowPass(int w, int h, Shader* shader);
 protected:
     void configuredRender(const FrameContext &ctx, const Camera & cam, Scene* scene) override;

 public:
     void updatePassSize(int w, int h) override;

     unsigned getOutput(int index) const override;
};


#endif //FOLDER_DIRECTIONALSHADOWPASS_H
