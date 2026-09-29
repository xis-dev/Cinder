#pragma once

#include "Camera.h"
#include "RenderPass.h"

#include "glm/vec2.hpp"

#include <string>
#include <array>
#include <unordered_map>


 class Scene;

class DirectionalShadowPass: public IRenderPassConfigurer<const Camera&, Scene*>
{

 public:
     float correctionStrength{0.9f};
     std::vector<float> m_cascadeSplits;

     glm::vec2 mainResolution{2048, 2048};
     unsigned texArray{};

     Shader* m_shader{nullptr};

     std::vector<std::string> cascadeUniformStrings;
     std::array<glm::mat4, 8> m_lightTransforms{};
     std::array<float, 8> m_cascadeTexelWorld{};

     void updateSplits(std::vector<float> &splits, float near, float far, float linearCorrection);

     FrameBuffer m_buffer;

     int numberOfSplits{4};

     glm::mat4 lightTransform{1.0f};

     DirectionalShadowPass() = default;
     DirectionalShadowPass(int w, int h, Shader* shader);
 protected:
     void configuredRender(const FrameContext &ctx, const Camera & cam, Scene* scene) override;

 public:
     void updatePassSize(int w, int h) override;

     unsigned getOutput(int index) const override;
};

