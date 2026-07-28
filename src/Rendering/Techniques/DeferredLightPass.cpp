//
// Created by PC on 26-Jul-26.
//

#include "../../../includes/Rendering/Techniques/DeferredLightPass.h"

#include "Camera.h"
#include "GBuffer.h"
#include "Quad.h"


DeferredLightPass::DeferredLightPass(int w, int h, Shader *shader): m_shader(shader)
{
    unsigned colour = Texture::createEmptyTex(w, h, GL_RGBA16F, GL_RGBA, GL_FLOAT);
    unsigned depthStencil = Texture::createEmptyRenderbuffer(w, h, GL_DEPTH24_STENCIL8);
    buffer = FrameBuffer(std::vector<unsigned>{colour});
    buffer.attachDepthBuffer(depthStencil, false, GL_DEPTH_STENCIL_ATTACHMENT);
    enable();
}

unsigned DeferredLightPass::getOutput(int index) const
{
    return buffer.getColourBuffer(index);
}

void DeferredLightPass::updatePassSize(int w, int h)
{
    buffer.updateColourBuffer(0, w, h, GL_RGBA, GL_FLOAT);
    buffer.updateDepthBuffer(w, h, GL_DEPTH_STENCIL_ATTACHMENT, GL_UNSIGNED_INT_24_8, GL_DEPTH24_STENCIL8, false);
}

void DeferredLightPass::configuredRender(const FrameContext &ctx, const SSAORenderPass &ssaoPass,
                                         unsigned shadowMapBuffer, Scene *scene, const Camera &cam)
{
    if (!scene || !ctx.gBuffer)
    {
        std::cout << "LIGHT_PASS: Irregular rendering, context either has no GBuffer or Scene.\n";
        return;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, buffer.getFrameBuffer());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_CULL_FACE);
    m_shader->use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ctx.gBuffer->gDepth);
    m_shader->setUniformi("u_GDepth", 0);



    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, ctx.gBuffer->gColorSpec);
    m_shader->setUniformi("u_GColorSpec", 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, ctx.gBuffer->gNormal);
    m_shader->setUniformi("u_GNormal", 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, ctx.gBuffer->gMaterial);
    m_shader->setUniformi("u_GMaterial", 3);


    m_shader->setUniformi("u_SSAOActive", ssaoPass.isEnabled());
    if (ssaoPass.isEnabled())
    {
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, ssaoPass.getOutput(0));
        m_shader->setUniformi("u_SSAO", 4);
    }


    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, shadowMapBuffer);
    m_shader->setUniformi("u_ShadowMap", 5);

    int pointMapStartIdx = 6;
    int loopIdx = 0;
    for (auto &[shadowCubemap, shadowMapTransforms]: scene->m_pointShadows | std::views::values)
    {
        glActiveTexture(GL_TEXTURE0 + pointMapStartIdx + loopIdx);
        glBindTexture(GL_TEXTURE_CUBE_MAP, shadowCubemap);
        m_shader->setUniformi(("t_PointMaps[" + std::to_string(loopIdx) + "]").c_str(),
                                     pointMapStartIdx + loopIdx);
        ++loopIdx;
    }

    scene->illuminate(*m_shader);
    for (const auto &projView: scene->dirLightTransforms)
    {
        m_shader->setUniformMat4("m_LightSpace", projView);
    }

    m_shader->setUniformVec3("u_CameraPosition", cam.getPosition());
    m_shader->setUniformVec3("u_ViewDirection", cam.getDirection());
    m_shader->setUniformi("u_Blinn", useBlinn);
    m_shader->setUniformf("u_NearPlane", cam.m_nearPlane);
    m_shader->setUniformf("u_FarPlane", cam.m_farPlane);
    m_shader->setUniformMat4("m_InvView", ctx.invViewMatrix);
    m_shader->setUniformMat4("m_InvProjection", ctx.invProjectionMatrix);


    Quad::draw();
}


