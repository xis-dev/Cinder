//
// Created by PC on 28-Jul-26.
//

#include "BloomPass.h"

#include "Quad.h"
#include "Shader.h"
#include "Texture.h"

BloomPass::BloomPass(int w, int h, Shader *bloomShader, Shader* blurShader): m_bloomShader(bloomShader), m_blurShader(blurShader)
{

    unsigned colourBuffer = Texture::createEmptyTex(w, h, GL_RGBA16F, GL_RGBA, GL_FLOAT);
    unsigned pingColour = Texture::createEmptyTex(w, h, GL_RGBA16F, GL_RGBA, GL_FLOAT);
    unsigned pongColour = Texture::createEmptyTex(w, h, GL_RGBA16F, GL_RGBA, GL_FLOAT);

    m_buffer = FrameBuffer({colourBuffer});
    m_pingBuffer = {{pingColour}};
    m_pongBuffer = {{pongColour}};

    enable();
}

void BloomPass::updatePassSize(int w, int h)
{
    m_buffer.    updateColourBuffer(0, w * bloomTextureScale, h * bloomTextureScale, GL_RGBA, GL_FLOAT);
    m_pingBuffer.updateColourBuffer(0, w * bloomTextureScale, h * bloomTextureScale, GL_RGBA, GL_FLOAT);
    m_pongBuffer.updateColourBuffer(0, w * bloomTextureScale, h * bloomTextureScale, GL_RGBA, GL_FLOAT);

    texelSize = {1. / (w), 1. / (h)};
    scaledTexelSize = {1. / (w * bloomTextureScale), 1. / (h * bloomTextureScale)};
}

unsigned BloomPass::getOutput(int index) const
{
    return m_pongBuffer.getColourBuffer();
}

void BloomPass::imguiRender()
{
    if (ImGui::TreeNode("Bloom"))
    {
        IRenderPassConfigurer::imguiRender();
        ImGui::DragFloat("Texture Relative Resolution", &bloomTextureScale, 0.1f);
        ImGui::DragFloat("Strength", &bloomStrength, 0.1f);
        ImGui::TreePop();
    }
}

void BloomPass::configuredRender(const FrameContext &ctx, unsigned bufferToBloom)
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_buffer.getFrameBuffer());
    glViewport(0, 0, ctx.frameWidth, ctx.frameHeight);
    glClear(GL_COLOR_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, bufferToBloom);

    m_bloomShader->use();

    m_bloomShader->setUniformi("t_BloomTexture", 0);
    m_bloomShader->setUniformVec2("u_OriginalTexelSize", texelSize);

    Quad::draw();

    glBindFramebuffer(GL_FRAMEBUFFER, m_pingBuffer.getFrameBuffer());
    glClear(GL_COLOR_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_buffer.getColourBuffer());

    m_blurShader->use();

    m_blurShader->setUniformi("t_TextureToBlur", 0);
    m_blurShader->setUniformVec2("u_TexelSize", scaledTexelSize);
    m_blurShader->setUniformi("horizontal", true);

    Quad::draw();

    glBindFramebuffer(GL_FRAMEBUFFER, m_pongBuffer.getFrameBuffer());
    glClear(GL_COLOR_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_pongBuffer.getColourBuffer());

    m_blurShader->setUniformi("t_TextureToBlur", 0);
    m_blurShader->setUniformi("horizontal", false);

    Quad::draw();

}
