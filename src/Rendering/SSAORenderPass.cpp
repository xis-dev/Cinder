//
// Created by PC on 24-Jul-26.
//

#include "../../includes/Rendering/Techniques/SSAORenderPass.h"

#include <iostream>
#include <random>

#include "GBuffer.h"
#include "imgui.h"
#include "Quad.h"
#include "Shader.h"
#include "Texture.h"
#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"

SSAORenderPass::SSAORenderPass(Shader* s_default, Shader* s_blur,int w, int h):
                                m_defaultShader(s_default), m_blurShader(s_blur), m_width(w), m_height(h)
{
    unsigned ssaoColour = Texture::createEmptyTex(w, h, GL_RED, GL_RED, GL_FLOAT);
    unsigned blurColour = Texture::createEmptyTex(w, h, GL_RED, GL_RED, GL_FLOAT);

    m_fbo = FrameBuffer(std::vector<unsigned>{ssaoColour});
    m_blurFBO = FrameBuffer(std::vector<unsigned>{blurColour});

    sampleKernel = generateSampleKernel();
    ssaoNoise = generateNoiseSamples();

    glGenTextures(1, &noiseTexture);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, ssaoNoise.data());

    glBindTexture(GL_TEXTURE_2D, 0);

    enable();
}



void SSAORenderPass::updatePassSize(int w, int h)
{
    m_width = w;
    m_height = h;

    m_fbo.updateColourBuffer(0, w, h, GL_RED, GL_FLOAT);
    m_blurFBO.updateColourBuffer(0, w, h, GL_RED, GL_FLOAT);

}



unsigned SSAORenderPass::getOutput(int index) const
{
    return m_blurFBO.getColourBuffer(index);
}

void SSAORenderPass::imguiRender()
{
    ImGui::Text("SSAO Settings:");
    IRenderPassConfigurer::imguiRender();
    ImGui::DragFloat("SSAO Strength", &strength, 0.1f);
    ImGui::DragFloat("Sample Radius", &sampleRadius, 0.1f);
}

std::vector<glm::vec3> SSAORenderPass::generateSampleKernel(int samples)
{
    std::uniform_real_distribution<float> randFloats(0.0f, 1.0f);
    std::default_random_engine generator(std::random_device{}());

    std::vector<glm::vec3> out;
    out.reserve(samples);
    for (int i = 0; i < samples; ++i)
    {
        // Vary samples between [-1,1], at z vary [0, 1] cause we're hemisphere sampling
        glm::vec3 sample {
            randFloats(generator) * 2.0 - 1.0,
            randFloats(generator) * 2.0 - 1.0,
            randFloats(generator)
        };

        float scale = (float)i/samples;
        scale = std::lerp(0.1f, 1.0f, scale * scale);

        sample = glm::normalize(sample);
        sample *= scale;
        out.push_back(sample);

    }

    return out;
}

std::vector<glm::vec3> SSAORenderPass::generateNoiseSamples(int samples)
{
    std::vector<glm::vec3> out;

    std::uniform_real_distribution<float> randFloats(0.0f, 1.0f);
    std::default_random_engine generator(std::random_device{}());
    for (int i = 0; i < samples; ++i)
    {
        glm::vec3 noise (
            randFloats(generator) * 2.0f - 1.0f,
            randFloats(generator) * 2.0f - 1.0f,
            0.0f
            );

        out.push_back(noise);
    }

    return out;
}

void SSAORenderPass::configuredRender(const FrameContext &context)
{
    if (!context.gBuffer)
    {
        std::cout << "SSAO_RENDER_PASS: Cannot render without GBuffer.\n";
        return;
    }

    glViewport(0, 0, m_width, m_height);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo.getFrameBuffer());

    glDisable(GL_CULL_FACE);

    glClear(GL_COLOR_BUFFER_BIT);


    m_defaultShader->use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, context.gBuffer->gDepth);
    m_defaultShader->setUniformi("t_GDepth", 0);


    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, context.gBuffer->gNormal);
    m_defaultShader->setUniformi("t_GNormal", 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    m_defaultShader->setUniformi("t_Noise", 2);

    m_defaultShader->setUniformMat4("m_View", context.viewMatrix);
    m_defaultShader->setUniformMat4("m_Projection", context.projectionMatrix);
    m_defaultShader->setUniformMat4("m_InvProjection", context.invProjectionMatrix);

    m_defaultShader->setUniformVec2("u_WindowSize", glm::vec2(m_width, m_height));
    m_defaultShader->setUniformVec2("u_NoiseSize", glm::vec2(4.0f));

    m_defaultShader->setUniformf("u_SampleRadius", sampleRadius);
    m_defaultShader->setUniformf("u_SampleBias", sampleBias);
    m_defaultShader->setUniformf("u_Strength", strength);


    m_defaultShader->setUniformVec3Array("u_SampleKernel", sampleKernel.data(), 64);

    Quad::draw();

    glBindFramebuffer(GL_FRAMEBUFFER, m_blurFBO.getFrameBuffer());
    glClear(GL_COLOR_BUFFER_BIT);

    m_blurShader->use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_fbo.getColourBuffer());
    m_blurShader->setUniformi("t_SSAO", 0);

    Quad::draw();
}
