#pragma once

#include "../FrameBuffer.h"
#include "RenderPass.h"

#include "vec3.hpp"
#include "ext/matrix_float4x4.hpp"

class Shader;

/**
 *
 */
class SSAORenderPass: public IRenderPassConfigurer<>
{
public:
    SSAORenderPass() = default;
    SSAORenderPass(Shader *s_default, Shader *s_blur, int w, int h);

private:
    FrameBuffer m_fbo{};
    FrameBuffer m_blurFBO{};

    Shader* m_defaultShader{nullptr};
    Shader* m_blurShader{nullptr};

    int m_width{};
    int m_height{};


    std::vector<glm::vec3> sampleKernel{};

    unsigned noiseTexture{};
    std::vector<glm::vec3> ssaoNoise{};
public:

    float strength = 1.0f;
    float sampleRadius = 0.5f;
    float sampleBias = 0.025f;


    void updatePassSize(int w, int h) override;

    /**
     * Render context buffer order:
     * GBuffer_Depth, GBuffer_Normal
     */

    unsigned getOutput(int index) const override;
    void imguiRender() override;
private:
    std::vector<glm::vec3> generateSampleKernel(int samples = 64);
    std::vector<glm::vec3> generateNoiseSamples(int samples = 16);

protected:
    void configuredRender(const FrameContext &context) override;

};


