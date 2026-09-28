//
// Created by PC on 28-Jul-26.
//

#include "HDRPass.h"

#include "Quad.h"
#include "Shader.h"
#include "Texture.h"

HDRPass::HDRPass(int w, int h, Shader *shader): m_shader(shader)
{
    unsigned colour = Texture::createEmptyTex(w, h, GL_RGBA16F, GL_RGBA, GL_FLOAT);
    m_buffer = {{colour}};

    enable();
}

void HDRPass::updatePassSize(int w, int h)
{
    m_buffer.updateColourBuffer(0, w, h, GL_RGBA, GL_FLOAT);
}

unsigned HDRPass::getOutput(int index) const
{
    return m_buffer.getColourBuffer(0);
}

void HDRPass::imguiRender()
{
    if (ImGui::TreeNode("HDR"))
    {
        IRenderPassConfigurer::imguiRender();
        ImGui::DragFloat("Exposure", &exposure, 0.1f);
        ImGui::TreePop();
    }
}

void HDRPass::configuredRender(const FrameContext &ctx, unsigned textureToApplyHDR, float gamma, const BloomPass &bloomPass)
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_buffer.getFrameBuffer());
    glViewport(0, 0, ctx.frameWidth, ctx.frameHeight);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_shader->use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureToApplyHDR);
    m_shader->setUniformi("u_HDRTexture", 0);

    m_shader->setUniformi("u_BloomEnabled", bloomPass.isEnabled());
    if (bloomPass.isEnabled())
    {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, bloomPass.getOutput(0));
        m_shader->setUniformi("u_BloomTexture", 1);
    }

    m_shader->setUniformf("u_HDRExposure", exposure);
    m_shader->setUniformf("u_Gamma", gamma);

    Quad::draw();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
