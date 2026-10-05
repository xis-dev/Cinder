//
// Created by PC on 26-Jul-26.
//

#include "../../../includes/Rendering/RenderPasses/DeferredLightPass.h"

#include "Camera.h"
#include "GBuffer.h"
#include "Quad.h"
#include "Components/LightComponent.h"


DeferredLightPass::DeferredLightPass(int w, int h, Shader *shader) : m_shader(shader)
{
    unsigned colour = Texture::createEmptyTex(w, h, GL_RGBA16F, GL_RGBA, GL_FLOAT);
    unsigned depthStencil = Texture::createEmptyRenderbuffer(w, h, GL_DEPTH24_STENCIL8);
    buffer = FrameBuffer(std::vector<unsigned>{colour});
    buffer.attachDepthBuffer(depthStencil, false, GL_DEPTH_STENCIL_ATTACHMENT);

    buffer.attachColourBuffer(Texture::createEmptyTex(1024, 1024, GL_RGBA16F, GL_RGBA, GL_FLOAT), true);
    buffer.attachColourBuffer(Texture::createEmptyTex(1024, 1024, GL_RGBA16F, GL_RGBA, GL_FLOAT), true);

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

void DeferredLightPass::imguiRender()
{
    if (ImGui::TreeNode("Light Pass"))
    {
        ImGui::Checkbox("Use Blinn-Phong", &useBlinn);
        ImGui::TreePop();
    }
}

void DeferredLightPass::configuredRender(const FrameContext &ctx, const SSAORenderPass &ssaoPass,
                                         unsigned shadowMapBuffer, Scene *scene, const Camera &cam,
                                         const DirectionalShadowPass& shadowPass)
{
    if (!scene || !ctx.gBuffer)
    {
        std::cout << "LIGHT_PASS: Irregular rendering, context either has no GBuffer or Scene.\n";
        return;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, buffer.getFrameBuffer());
    glViewport(0, 0, ctx.frameWidth, ctx.frameHeight);
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
    glBindTexture(GL_TEXTURE_2D_ARRAY, shadowPass.getOutput(0));
    m_shader->setUniformi("u_ShadowMap", 5);

    const auto lights = scene->getEntitiesByComponents<LightComponent>();

    int pointMapStartIdx = 6;

    int dirCount{};
    int pointCount{};
    int spotCount{};

    for (const auto& light: lights)
    {
        const auto* lightComp = light->getComponent<LightComponent>();

        switch (lightComp->m_type)
        {
            case LightComponent::Type::Directional: {

                const std::string dirUniformStr{ "u_DirectionalLights[" + std::to_string(dirCount++) + "]." };

                m_shader->setUniformVec3((dirUniformStr + "direction").c_str(), glm::normalize(get<DirectionalLightSettings>(lightComp->m_settings).m_direction));
                m_shader->setUniformVec3((dirUniformStr + "color").c_str(), get<DirectionalLightSettings>(lightComp->m_settings).m_color);
                m_shader->setUniformf((dirUniformStr + "intensity").c_str(), get<DirectionalLightSettings>(lightComp->m_settings).m_intensity);
                break;
            }

            case LightComponent::Type::Point: {
                glActiveTexture(GL_TEXTURE0 + pointMapStartIdx + pointCount);
                glBindTexture(GL_TEXTURE_CUBE_MAP, lightComp->m_shadow.m_shadowMap);
                m_shader->setUniformi(("t_PointMaps[" + std::to_string(pointCount) + "]").c_str(),
                                      pointMapStartIdx + pointCount);

                const std::string pointUniformStr{ "u_PointLights[" + std::to_string(pointCount) + "]." };

                m_shader->setUniformf((pointUniformStr + "radius").c_str(), get<PointLightSettings>(lightComp->m_settings).m_attenuationRadius);

                m_shader->setUniformVec3((pointUniformStr + "color").c_str(), get<PointLightSettings>(lightComp->m_settings).m_color);
                m_shader->setUniformf((pointUniformStr + "intensity").c_str(), get<PointLightSettings>(lightComp->m_settings).m_intensity);
                m_shader->setUniformVec3((pointUniformStr + "position").c_str(), Transform::getWorldPosition(*light));

                ++pointCount;

                break;
            }

            case LightComponent::Type::Spot: {
                const std::string spotUniformStr = "u_SpotLights[" + std::to_string(spotCount++) + "].";

                m_shader->setUniformf((spotUniformStr + "innerCutoff").c_str(), get<SpotLightSettings>(lightComp->m_settings).m_innerCutoff);
                m_shader->setUniformf((spotUniformStr + "outerCutoff").c_str(), get<SpotLightSettings>(lightComp->m_settings).m_outerCutoff);

                m_shader->setUniformVec3((spotUniformStr + "color").c_str(), get<SpotLightSettings>(lightComp->m_settings).m_color);
                m_shader->setUniformf((spotUniformStr + "intensity").c_str(), get<SpotLightSettings>(lightComp->m_settings).m_intensity);

                m_shader->setUniformVec3((spotUniformStr + "direction").c_str(), glm::normalize(get<SpotLightSettings>(lightComp->m_settings).m_direction));
                m_shader->setUniformVec3((spotUniformStr + "position").c_str(), Transform::getWorldPosition(*light));

                break;
            }
        }
    }

    m_shader->setUniformi("u_DirLightCount", dirCount);
    m_shader->setUniformi("u_PointLightCount", pointCount);
    m_shader->setUniformi("u_SpotLightCount", spotCount);


    m_shader->setUniformi("u_CascadeMapCount", shadowPass.numberOfSplits);
    for (int i = 0; i < shadowPass.numberOfSplits; ++i)
    {
        const std::string lsUniformStr = "m_LightSpace[" + std::to_string(i) + "]";
        const std::string tsUniformStr = "m_CascadeTexelWorld[" + std::to_string(i) + "]";

        m_shader->setUniformf(shadowPass.cascadeUniformStrings[i].c_str(), shadowPass.m_cascadeSplits[i]);

        m_shader->setUniformMat4(lsUniformStr.c_str(), shadowPass.m_lightTransforms[i]);
        m_shader->setUniformMat4(tsUniformStr.c_str(), shadowPass.m_cascadeTexelWorld[i]);
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
