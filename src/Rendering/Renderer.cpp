#include "includes/Rendering/Renderer.h"

#include "AssetManager.h"
#include "Camera.h"
#include "includes/Resources/Texture.h"
#include "includes/Primitives/Skybox.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <random>
#include <ranges>

#include "Engine.h"
#include "Quad.h"
#include "GLFW/glfw3.h"


#include "imgui.h"
#include "Components/LightComponent.h"

#include "gtc/type_ptr.hpp"
#include "gtx/norm.hpp"
#include "RenderPasses/BloomPass.h"

#include "Entity.h"
#include "Components/MeshComponent.h"

void Renderer::init(GLFWwindow *win, AssetManager *manager, Scene *scene, int width, int height)
{
    // TODO: Add static assertions
    WINDOW = win;
    ASSET_MANAGER = manager;
    CURRENT_SCENE = scene;

    m_renderWidth = width;
    m_renderHeight = height;

    glCullFace(GL_BACK);


    createSkybox();
    cubeMapTex = Texture::createCubemap(textures_faces);

    shadowTex = Texture::createEmptyTex(1920, 1080, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT);
    shadowFBO = create2DShadowFBO(shadowTex);


    pointShadowTex = Texture::createEmptyCubemap(2048, 2048, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT);
    pointShadowFBO = createCubemapShadowFBO(pointShadowTex);


    m_GBuffer.setup(width, height);
    m_SSAOPass = SSAORenderPass(ASSET_MANAGER->shaders.get("ssaoShader"), ASSET_MANAGER->shaders.get("ssaoBlur"), width, height);
    m_LightPass = DeferredLightPass(width, height,ASSET_MANAGER->shaders.get("deferredLightPass"));
    m_BloomPass = BloomPass(width, height, ASSET_MANAGER->shaders.get("bloom"), ASSET_MANAGER->shaders.get("bloomBlur"));
    m_HDRPass = HDRPass(width, height, ASSET_MANAGER->shaders.get("HDR"));
    m_DirShadowPass = DirectionalShadowPass(width, height, ASSET_MANAGER->shaders.get("shadowMap"));

    m_renderPasses.push_back(&m_SSAOPass);
    m_renderPasses.push_back(&m_LightPass);
    m_renderPasses.push_back(&m_BloomPass);
    m_renderPasses.push_back(&m_HDRPass);
    m_renderPasses.push_back(&m_DirShadowPass);

    for (int i = 0; i < 6; ++i) shadowMatNames[i] = "u_ShadowMatrices[" + std::to_string(i) + "]";

}


void Renderer::changeViewportSize(int w, int h)
{
    m_renderWidth = w;
    m_renderHeight = h;

    updateRenderComponents(w, h);
}

void Renderer::imguiRender()
{
    for (auto* pass: m_renderPasses)
    {
        pass->imguiRender();
    }
    ImGui::DragFloat("Gamma", &gamma);
	ImGui::Checkbox("Grid", &drawGrid);
    ImGui::Checkbox("Draw CubeMap", &cubeMapEnabled);
    ImGui::Checkbox("Enable Backface Culling", &cullBackface);
    ImGui::Checkbox("Draw Wireframe", &drawWireframe);
}


void Renderer::updateRenderComponents(int w, int h)
{
    m_GBuffer.update(w, h);

    for (auto* pass: m_renderPasses)
    {
        pass->updatePassSize(w, h);
    }
}



void Renderer::createSkybox()
{
    ASSET_MANAGER->shaders.add(Shader{"assets/Shaders/skybox/skybox.vert", "assets/Shaders/skybox/skybox.frag"},
                               "skybox");
    glGenBuffers(1, &skyBoxVBO);
    glGenVertexArrays(1, &skyBoxVAO);
    glBindVertexArray(skyBoxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyBoxVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(SkyBox::vertices), SkyBox::vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}


void Renderer::drawSkybox(const Camera &cam)
{
    auto skyBoxShader = ASSET_MANAGER->shaders.get("skybox");
    skyBoxShader->use();
    skyBoxShader->setUniformMat4("u_Projection", cam.getProjectionMatrix());
    skyBoxShader->setUniformMat4("u_View", glm::mat4(glm::mat3(cam.getViewMatrix())));
    skyBoxShader->setUniformi("u_Skybox", 0);
    skyBoxShader->setUniformf("u_Gamma", gamma);
    glBindVertexArray(skyBoxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTex);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

unsigned Renderer::createFBO(unsigned *colorTexts, int colorTexCount, unsigned depthStencil)
{
    std::vector<unsigned> attachments{};
    unsigned id;
    glGenFramebuffers(1, &id);
    glBindFramebuffer(GL_FRAMEBUFFER, id);

    for (int i = 0; i < colorTexCount; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, colorTexts[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorTexts[i], 0);
        attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
    }
    if (depthStencil > 0)
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthStencil);

    glDrawBuffers(attachments.size(), attachments.data());
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Incomplete Framebuffer. \n";
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return id;
}



unsigned Renderer::create2DShadowFBO(unsigned depthTex)
{
    unsigned id;
    glGenFramebuffers(1, &id);
    glBindFramebuffer(GL_FRAMEBUFFER, id);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Complete. \n";
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return id;
}

unsigned Renderer::createCubemapShadowFBO(unsigned depthCubemap)
{
    unsigned id;
    glGenFramebuffers(1, &id);
    glBindFramebuffer(GL_FRAMEBUFFER, id);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    std::cout << "Status: " << glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Cubemap Complete. \n";
    } else
    {
        std::cout << "Complete: " << GL_FRAMEBUFFER_COMPLETE << std::endl;
        std::cout << "InCompleteaTT: " << GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT << std::endl;
        std::cout << "Complete: " << GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return id;
}


void Renderer::renderScene(const Camera &cam, unsigned fboToRenderTo, const int sceneW, const int sceneH)
{
    // Disable blending for deferred


    glm::vec3 camPosition = cam.getPosition();
    glm::mat4 projectionMat = cam.getProjectionMatrix();
    glm::mat4 invProjection = glm::inverse(projectionMat);
    glm::mat4 viewMat = cam.getViewMatrix();
    glm::mat4 invView = glm::inverse(viewMat);
    glm::mat4 vpMat = projectionMat * viewMat;

    FrameContext currentFrameContext{};
    currentFrameContext.gBuffer = &m_GBuffer;
    currentFrameContext.frameWidth = m_renderWidth;
    currentFrameContext.frameHeight = m_renderHeight;
    currentFrameContext.viewMatrix = viewMat;
    currentFrameContext.invViewMatrix = invView;
    currentFrameContext.projectionMatrix = projectionMat;
    currentFrameContext.invProjectionMatrix = invProjection;

    // Render directional shadow map
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    m_DirShadowPass.tryConfiguredRender(currentFrameContext, cam, CURRENT_SCENE);


    glDisable(GL_BLEND);
    glBindFramebuffer(GL_FRAMEBUFFER, fboToRenderTo);
    glViewport(0, 0, sceneW, sceneH);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const auto lights = CURRENT_SCENE->getEntitiesByComponents<LightComponent>();
    const auto meshEnts = CURRENT_SCENE->getEntitiesByComponents<MeshComponent>();

    // TODO: Move to some light system

    // TODO: Material batches(or at least shader batches again)
    for (const auto& ent: meshEnts)
    {
        const auto* meshComp = ent->getComponent<MeshComponent>();

        for (const auto& modelSet: meshComp->m_model->getMeshes())
        {
           const Material* mat = ASSET_MANAGER->materials.get(modelSet.mat);
           const Shader* shader = ASSET_MANAGER->shaders.get(mat->getShader());

            shader->use();


            // Shared matrices
            shader->setUniformMat4("u_VPMatrix", vpMat);
            shader->setUniformMat4("u_ViewMatrix", viewMat);
            shader->setUniformMat4("u_ProjectionMatrix", projectionMat);

            // Camera uniforms
            shader->setUniformVec3("u_CameraPosition", camPosition);
            shader->setUniformVec3("u_ViewDirection", cam.getDirection());

            shader->setUniformf("u_ParallaxHeightScale", parallaxScale);

            auto globalTransform = Transform::getGlobalTransform(*ent);
            shader->setUniformMat4("m_Model", Transform::getGlobalTransform(*ent));

            shader->setUniformMat4("m_MVP", vpMat * globalTransform);

            const auto textures = mat->getTextures();
            int textureUnit = 0;
            int diffuseNr = 0;
            int specularNr = 0;
            int normalNr = 0;
            int heightNr = 0;
            if (!textures.empty())
            {
                for (auto texture: textures)
                {
                    auto *currentTex = ASSET_MANAGER->textures.get(texture);
                    std::string uniformStr;

                    switch (currentTex->getType())
                    {
                        case Texture::Specular:
                            uniformStr = "t_Specular[" + std::to_string(specularNr) + "]";
                            ++specularNr;
                            break;

                        case Texture::Normal:
                            uniformStr = "t_Normal[" + std::to_string(normalNr) + "]";
                            ++normalNr;
                            break;

                        case Texture::Height:
                            uniformStr = "t_Height[" + std::to_string(heightNr) + "]";
                            ++heightNr;
                            break;
                        case Texture::Diffuse:

                        default:
                            uniformStr = "t_Diffuse[" + std::to_string(diffuseNr) + "]";
                            ++diffuseNr;
                            break;
                    }
                    shader->setUniformi(uniformStr.c_str(), textureUnit);

                    glActiveTexture(GL_TEXTURE0 + textureUnit);
                    currentTex->use();
                    ++textureUnit;
                }
            }
            shader->setUniformi("u_DiffuseMapCount", diffuseNr);
            shader->setUniformi("u_SpecularMapCount", specularNr);
            shader->setUniformi("u_NormalMapCount", normalNr);
            shader->setUniformi("u_HeightMapCount", heightNr);

            std::string materialUniformBase = "u_Material.";
            shader->setUniformVec3((materialUniformBase + "albedo").c_str(), mat->getColor());
            shader->setUniformf((materialUniformBase + "ambient").c_str(), mat->getAmbience());
            shader->setUniformf((materialUniformBase + "diffuse").c_str(), mat->getDiffuse());
            shader->setUniformf((materialUniformBase + "specular").c_str(), mat->getSpecular());
            shader->setUniformf((materialUniformBase + "shininess").c_str(), mat->getShininess());


            modelSet.mesh.draw();

            // Unbind textures
            for (int i = 0; i <= textureUnit; ++i)
            {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, 0);
                glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
            }
        }
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    m_SSAOPass.tryConfiguredRender(currentFrameContext);
    m_LightPass.tryConfiguredRender(currentFrameContext, m_SSAOPass, m_DirShadowPass.getOutput(0), CURRENT_SCENE, cam, m_DirShadowPass
        );

    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);

    glBindFramebuffer(GL_FRAMEBUFFER, m_GBuffer.frameBufferHolder.getFrameBuffer());
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_LightPass.buffer.getFrameBuffer());
    glBlitFramebuffer(0, 0, sceneW, sceneH, 0, 0, sceneW, sceneH, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, m_LightPass.buffer.getFrameBuffer());
    if (cubeMapEnabled)
    {
        glDepthFunc(GL_LEQUAL);
        glDepthMask(GL_FALSE);
        glDisable(GL_CULL_FACE);

        drawSkybox(cam);

        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);
    }

    glDisable(GL_CULL_FACE);
    // Draw floor grid
    if (drawGrid)
    {
        if (auto *gridShader = ASSET_MANAGER->shaders.get("grid"))
        {
            gridShader->use();
            gridShader->setUniformMat4("u_VPMatrix", (cam.getProjectionMatrix() * cam.getViewMatrix()));
            gridShader->setUniformVec3("u_CameraPosition", cam.getPosition());
            gridShader->setUniformf("u_Gamma", gamma);

            Quad::draw();
        }
    }

    glDepthFunc(GL_ALWAYS);

    // Render entity icons(if they exist)
    if (auto iconShader = ASSET_MANAGER->shaders.get("icon"))
    {
        glm::mat4 view = cam.getViewMatrix();
        iconShader->use();
        glm::mat4 projection = cam.getProjectionMatrix();

        glm::vec3 cameraRightWorldSpace = glm::vec3{view[0][0], view[1][0], view[2][0]};
        glm::vec3 cameraUpWorldSpace = glm::vec3{view[0][1], view[1][1], view[2][1]};
        iconShader->setUniformVec3("u_CameraRight_WorldSpace", cameraRightWorldSpace);

        iconShader->setUniformVec3("u_CameraUp_WorldSpace", cameraUpWorldSpace);

        for (const auto& entity: CURRENT_SCENE->getEntities())
        {
            for (const auto& compID : entity->getComponents() | std::views::keys)
            {
                if (const auto* tex = IconRegistry::tryGetIcon(compID))
                {
                    iconShader->use();

                    iconShader->setUniformVec3("u_ObjectPosition", (glm::vec3) Transform::getWorldPosition(*entity));
                    iconShader->setUniformMat4("u_ProjectionMatrix", projection);
                    iconShader->setUniformMat4("u_ViewMatrix", view);
                    iconShader->setUniformf("u_Gamma", gamma);

                    glActiveTexture(GL_TEXTURE0);
                    tex->use();
                    iconShader->setUniformi("u_iconImage", 0);
                    Quad::draw();
                }
            }
        }
    }

    glDisable(GL_CULL_FACE);
    m_BloomPass.tryConfiguredRender(currentFrameContext, m_LightPass.getOutput(0));
    m_HDRPass.tryConfiguredRender(currentFrameContext, m_LightPass.getOutput(0),  gamma, m_BloomPass);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glEnable(GL_CULL_FACE);
}

void Renderer::renderShadowMap()
{
    // glViewport(0, 0, 1920, 1080);
    // glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    // glClear(GL_DEPTH_BUFFER_BIT);
    //
    // auto *shadowMapShader = ASSET_MANAGER->shaders.get("shadowMap");
    // if (shadowMapShader)
    // {
    //     shadowMapShader->use();
    //     for (const auto &projView: CURRENT_SCENE->dirLightTransforms)
    //     {
    //         shadowMapShader->setUniformMat4("u_LightProjView", projView);
    //     }
    // }
    //
    // for (const auto &entity: CURRENT_SCENE->m_meshEnts)
    // {
    //     for (auto &modelSet: entity->getModel()->getMeshes())
    //     {
    //         shadowMapShader->setUniformMat4("u_Model", entity->getGlobalTransformMatrix());
    //         modelSet.mesh.draw();
    //     }
    // }
}

// TODO: Pass in lights and mesh entities to avoid looking for them again
void Renderer::renderPointMap(Scene *currentScene)
{
    glBindFramebuffer(GL_FRAMEBUFFER, pointShadowFBO);
    auto *pointMapShader = ASSET_MANAGER->shaders.get("pointMap");
    pointMapShader->use();

    const auto meshEnts = currentScene->getEntitiesByComponents<MeshComponent>();

    const auto lights = currentScene->getEntitiesByComponents<LightComponent>();
    for (const auto* light: lights)
    {
        auto* lightComponent = light->getComponent<LightComponent>();
        if (lightComponent->m_type != LightComponent::Type::Point) continue;

        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, lightComponent->m_shadow.m_shadowMap, 0);
        glClear(GL_DEPTH_BUFFER_BIT);

        pointMapShader->setUniformVec3("u_LightPos", light->transform->m_position);
        pointMapShader->setUniformf("u_FarPlane", get<PointLightSettings>(lightComponent->m_settings).m_attenuationRadius);

        auto mapTransforms = getPointMapMatrices(*light, m_renderWidth, m_renderHeight);
        for (int i = 0; i < 6; ++i)
        {
            pointMapShader->setUniformMat4(shadowMatNames[i].c_str(), mapTransforms[i]);
        }


        for (const auto* entity: meshEnts)
        {
            const auto* meshComp = entity->getComponent<MeshComponent>();
            for (const auto &[mesh, mat] : meshComp->m_model->getMeshes())
            {
                pointMapShader->setUniformMat4("u_Model", Transform::getTransformMatrix(*(entity->transform)));
                mesh.draw();
            }
        }
    }

}

std::array<glm::mat4, 6> Renderer::getPointMapMatrices(const Entity& light, int w, int h)
{
    std::array<glm::mat4, 6> out{};
    const auto* lightComponent = light.getComponent<LightComponent>();
    if (!lightComponent) return out;

    float aspect = (float)w / (float)h;
    float near = 1.0f;
    glm::mat4 shadow_proj = glm::perspective(glm::radians(90.0f), aspect, near, std::get<PointLightSettings>(lightComponent->m_settings).m_attenuationRadius);

    glm::vec3 lightPos = light.transform->m_position;
    out[0] = shadow_proj *
    glm::lookAt(lightPos, lightPos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
    out[1] = shadow_proj *
        glm::lookAt(lightPos, lightPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
    out[2] = shadow_proj *
        glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0));
    out[3] = shadow_proj *
        glm::lookAt(lightPos, lightPos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0));
    out[4] = shadow_proj *
        glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0));
    out[5] = shadow_proj *
        glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0));

    return out;
}


unsigned Renderer::getFinalSceneTexture()
{
    return m_HDRPass.isEnabled() ? m_HDRPass.getOutput(0) : m_LightPass.getOutput(0);
}


void Renderer::render(const Camera &cam)
{

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);



    if (drawWireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }


    glEnable(GL_CULL_FACE);


    glCullFace(GL_FRONT);

    //renderShadowMap();

    glViewport(0, 0, 2048, 2048);
    renderPointMap(CURRENT_SCENE);

    glCullFace(GL_BACK);
    if (cullBackface)
    {
        glEnable(GL_CULL_FACE);
    } else
    {
        glDisable(GL_CULL_FACE);
    }

    renderScene(cam, m_GBuffer.frameBufferHolder.getFrameBuffer(), m_renderWidth, m_renderHeight);
}
