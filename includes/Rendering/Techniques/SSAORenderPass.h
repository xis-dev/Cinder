#pragma once

#include "../FrameBuffer.h"
#include "vec3.hpp"
#include "ext/matrix_float4x4.hpp"

class Shader;

// TODO: Once done, strip out general parts and make RenderPass base class
class SSAORenderPass
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
    bool enabled{};
public:

    float strength = 1.0f;
    float sampleRadius = 0.5f;
    float sampleBias = 0.025f;

    void enable();
    void disable();

    bool isEnabled();

    void updatePassSize(int w, int h);

    void render(unsigned gDepth, unsigned gNormal, const glm::mat4 &viewMat, const glm::mat4 &projMat, const glm::
                mat4 &invProjMat);

    unsigned getOuputBuffer(int index = 0);
    void imguiDraw();
private:
    std::vector<glm::vec3> generateSampleKernel(int samples = 64);
    std::vector<glm::vec3> generateNoiseSamples(int samples = 16);

};


