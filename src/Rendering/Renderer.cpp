#include "includes/Rendering/Renderer.h"

#include "AssetManager.h"
#include "Camera.h"
#include "includes/Resources/Texture.h"
#include "includes/Primitives/Skybox.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <random>

#include "Engine.h"
#include "Quad.h"
#include "GLFW/glfw3.h"


#include "gtc/type_ptr.hpp"
#include "gtx/norm.hpp"

void Renderer::init(GLFWwindow *win, AssetManager *manager, Scene *scene, int width, int height)
{
    // TODO: Add static assertions
    WINDOW = win;
    ASSET_MANAGER = manager;
    CURRENT_SCENE = scene;

    renderWidth = width;
    renderHeight = height;

    glCullFace(GL_BACK);


    createSkybox();
    cubeMapTex = Texture::createCubemap(textures_faces);

    shadowTex = Texture::createEmptyTex(1920, 1080, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT);
    shadowFBO = create2DShadowFBO(shadowTex);


    pointShadowTex = Texture::createEmptyCubemap(2048, 2048, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT);
    pointShadowFBO = createCubemapShadowFBO(pointShadowTex);


    glGenFramebuffers(2, pingPongFBOs);
    glGenTextures(2, pingPongColorBuffers);
    createPingPongFBOs();
    hdrColorTexs[0] = Texture::createEmptyTex(width, height, GL_RGBA16F, GL_RGBA);

    // hdrColorTexs[1] = Texture::createEmptyTex(width, height, GL_RGBA16F, GL_RGBA);
    hdrDepthStencil = Texture::createEmptyTex(width, height, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL,
                                              GL_UNSIGNED_INT_24_8);
    hdrFBO = createFBO(hdrColorTexs, 1);



    bloomColor = Texture::createEmptyTex(width, height, GL_RGBA16F, GL_RGBA, GL_FLOAT);
    bloomFBO = createFBO(&bloomColor, 1);


    m_GBuffer.setup(width, height);
    m_SSAOPass = SSAORenderPass(ASSET_MANAGER->shaders.get("ssaoShader"), ASSET_MANAGER->shaders.get("ssaoBlur"), width, height);
    m_LightPass = DeferredLightPass(width, height,ASSET_MANAGER->shaders.get("deferredLightPass"));

    for (int i = 0; i < 6; ++i) shadowMatNames[i] = "u_ShadowMatrices[" + std::to_string(i) + "]";

}


void Renderer::changeViewportSize(int w, int h)
{
    renderWidth = w;
    renderHeight = h;

    updateRenderComponents(w, h);
    createPingPongFBOs();
}


void Renderer::updateRenderComponents(int w, int h)
{
    m_GBuffer.update(w, h);
    // TODO: Wrap into holding render passes and call in loop
    m_SSAOPass.updatePassSize(w, h);
    m_LightPass.updatePassSize(w, h);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Incomplete deferred Framebuffer. \n";
    }

    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

    glBindTexture(GL_TEXTURE_2D, hdrColorTexs[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hdrColorTexs[0], 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Incomplete hdr Framebuffer. \n";
    }

    glBindFramebuffer(GL_FRAMEBUFFER, bloomFBO);

    glBindTexture(GL_TEXTURE_2D, bloomColor);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, int(w * 0.5), int(h * 0.5), 0, GL_RGBA, GL_FLOAT, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bloomColor, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Incomplete bloom Framebuffer. \n";
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

void Renderer::createPingPongFBOs()
{
    for (int i = 0; i < 2; ++i)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingPongFBOs[i]);
        glBindTexture(GL_TEXTURE_2D, pingPongColorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, renderWidth * 0.5, renderHeight * 0.5, 0, GL_RGBA, GL_FLOAT,
                     nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingPongColorBuffers[i], 0);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

void Renderer::setupPointMatrices(PointLight *light, const int w, const int h)
{
    shadowTransforms.clear();
    float aspect = (float) w / (float) h;
    float near = 1.0f;
    float far = 25.0f;
    farPlane = far;
    glm::mat4 shadow_proj = glm::perspective(glm::radians(90.0f), aspect, near, far);

    glm::vec3 lightPos = light->getWorldPosition();
    shadowTransforms.push_back(shadow_proj *
                               glm::lookAt(lightPos, lightPos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
    shadowTransforms.push_back(shadow_proj *
                               glm::lookAt(lightPos, lightPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
    shadowTransforms.push_back(shadow_proj *
                               glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
    shadowTransforms.push_back(shadow_proj *
                               glm::lookAt(lightPos, lightPos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
    shadowTransforms.push_back(shadow_proj *
                               glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
    shadowTransforms.push_back(shadow_proj *
                               glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));
}

void Renderer::renderScene(const Camera &cam, unsigned fboToRenderTo, const int sceneW, const int sceneH)
{
    // Disable blending for deferred

    glDisable(GL_BLEND);
    glBindFramebuffer(GL_FRAMEBUFFER, fboToRenderTo);
    glViewport(0, 0, sceneW, sceneH);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::vec3 camPosition = cam.getPosition();
    glm::mat4 projectionMat = cam.getProjectionMatrix();
    glm::mat4 invProjection = glm::inverse(projectionMat);
    glm::mat4 viewMat = cam.getViewMatrix();
    glm::mat4 invView = glm::inverse(viewMat);
    glm::mat4 vpMat = projectionMat * viewMat;

    FrameContext currentFrameContext{};
    currentFrameContext.gBuffer = &m_GBuffer;
    currentFrameContext.viewMatrix = viewMat;
    currentFrameContext.invViewMatrix = invView;
    currentFrameContext.projectionMatrix = projectionMat;
    currentFrameContext.invProjectionMatrix = invProjection;

    for (Shader *shader: CURRENT_SCENE->m_renderBatches | std::views::keys)
    {
        shader->use();

        // Shared matrices
        shader->setUniformMat4("u_VPMatrix", vpMat);
        shader->setUniformMat4("u_ViewMatrix", viewMat);
        shader->setUniformMat4("u_ProjectionMatrix", projectionMat);

        // Camera uniforms
        shader->setUniformVec3("u_CameraPosition", camPosition);
        shader->setUniformVec3("u_ViewDirection", cam.getDirection());


        shader->setUniformf("u_ParallaxHeightScale", parallaxScale);

        // TODO: Batch per material
        for (const auto &[modelSet, entity]: CURRENT_SCENE->m_renderBatches[shader])
        {
            Material *mat = ASSET_MANAGER->materials.get(modelSet->mat);


            shader->setUniformMat4("m_Model", entity->getGlobalTransformMatrix());
            shader->setUniformMat4("m_MVP", vpMat * entity->getGlobalTransformMatrix());


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


            modelSet->mesh.draw();

            for (int i = 0; i <= textureUnit; ++i)
            {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, 0);
                glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
            }
        }
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    if (useSSAO)
    {
        m_SSAOPass.tryConfiguredRender(currentFrameContext);
    }

    m_LightPass.tryConfiguredRender(currentFrameContext, m_SSAOPass, shadowTex, CURRENT_SCENE, cam);


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

    glEnable(GL_CULL_FACE);
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

        for (auto &entity: CURRENT_SCENE->getEntities())
        {
            if (entity->hasIcon())
            {
                iconShader->use();

                iconShader->setUniformVec3("u_ObjectPosition", (glm::vec3) entity->getWorldPosition());
                iconShader->setUniformMat4("u_ProjectionMatrix", projection);
                iconShader->setUniformMat4("u_ViewMatrix", view);
                iconShader->setUniformf("u_Gamma", gamma);

                glActiveTexture(GL_TEXTURE0);
                entity->tryGetIcon()->use();
                iconShader->setUniformi("u_iconImage", 0);
                Quad::draw();
            }
        }
    }

    glDisable(GL_CULL_FACE);

    if (bloom)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, bloomFBO);
        glViewport(0, 0, renderWidth * 0.5, renderHeight * 0.5);
        glClear(GL_COLOR_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_LightPass.getOutput(0));

        auto *bloomShader = ASSET_MANAGER->shaders.get("bloom");
        bloomShader->use();

        bloomShader->setUniformi("t_BloomTexture", 0);
        bloomShader->setUniformVec2("u_OriginalTexelSize", glm::vec2(1.0 / renderWidth, 1.0 / renderHeight));

        Quad::draw();

        glBindFramebuffer(GL_FRAMEBUFFER, pingPongFBOs[0]);
        glClear(GL_COLOR_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, bloomColor);

        auto *bloomBlurShader = ASSET_MANAGER->shaders.get("bloomBlur");
        bloomBlurShader->use();

        bloomBlurShader->setUniformi("t_TextureToBlur", 0);
        bloomBlurShader->setUniformVec2("u_TexelSize", glm::vec2(1. / (renderWidth * 0.5), 1. / (renderHeight * 0.5)));
        bloomBlurShader->setUniformi("horizontal", true);

        Quad::draw();

        glBindFramebuffer(GL_FRAMEBUFFER, pingPongFBOs[1]);
        glClear(GL_COLOR_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, pingPongColorBuffers[0]);

        bloomBlurShader->setUniformi("t_TextureToBlur", 0);
        bloomBlurShader->setUniformi("horizontal", false);

        Quad::draw();

        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        glViewport(0, 0, renderWidth, renderHeight);
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_LightPass.getOutput(0));

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pingPongColorBuffers[1]);

        auto *hdrShader = ASSET_MANAGER->shaders.get("HDR");
        hdrShader->use();
        hdrShader->setUniformi("u_HDRTexture", 0);
        hdrShader->setUniformi("u_BloomTexture", 1);
        hdrShader->setUniformVec2("u_DownsampledTexelSize",
                                  glm::vec2(1. / (renderWidth * 0.5), 1. / (renderHeight * 0.5)));
        hdrShader->setUniformf("u_HDRExposure", hdrExposure);
        hdrShader->setUniformf("u_Gamma", gamma);

        Quad::draw();
    } else
    {
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        glViewport(0, 0, renderWidth, renderHeight);
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_LightPass.getOutput(0));

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_LightPass.getOutput(0));

        auto *hdrShader = ASSET_MANAGER->shaders.get("HDR");
        hdrShader->use();
        hdrShader->setUniformi("u_HDRTexture", 0);
        hdrShader->setUniformi("u_BloomTexture", 1);
        hdrShader->setUniformVec2("u_DownsampledTexelSize",
                                  glm::vec2(1. / (renderWidth * 0.5), 1. / (renderHeight * 0.5)));
        hdrShader->setUniformf("u_HDRExposure", hdrExposure);
        hdrShader->setUniformf("u_Gamma", gamma);

        Quad::draw();
    }


    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glEnable(GL_CULL_FACE);
}

void Renderer::renderShadowMap()
{
    glViewport(0, 0, 1920, 1080);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    auto *shadowMapShader = ASSET_MANAGER->shaders.get("shadowMap");
    if (shadowMapShader)
    {
        shadowMapShader->use();
        for (const auto &projView: CURRENT_SCENE->dirLightTransforms)
        {
            shadowMapShader->setUniformMat4("u_LightProjView", projView);
        }
    }

    for (const auto &entity: CURRENT_SCENE->m_meshEnts)
    {
        for (auto &modelSet: entity->getModel()->getMeshes())
        {
            shadowMapShader->setUniformMat4("u_Model", entity->getGlobalTransformMatrix());
            modelSet.mesh.draw();
        }
    }
}

void Renderer::renderPointMap(Scene *currentScene)
{
    glBindFramebuffer(GL_FRAMEBUFFER, pointShadowFBO);
    auto *pointMapShader = ASSET_MANAGER->shaders.get("pointMap");
    pointMapShader->use();
    currentScene->setupPointMatrices(2048, 2048);
    for (std::pair<PointLight *, PointShadow> ps: currentScene->m_pointShadows)
    {
        auto *light = ps.first;
        auto &shadow = ps.second;
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadow.shadowCubemap, 0);
        glClear(GL_DEPTH_BUFFER_BIT);
        pointMapShader->setUniformVec3("u_LightPos", light->getWorldPosition());
        pointMapShader->setUniformf("u_FarPlane", light->m_radius);

        for (int i = 0; i < 6; ++i)
        {
            pointMapShader->setUniformMat4(shadowMatNames[i].c_str(), shadow.shadowMapTransforms[i]);
        }

        for (const auto &entity: CURRENT_SCENE->m_meshEnts)
        {
            for (auto &modelSet: entity->getModel()->getMeshes())
            {
                pointMapShader->setUniformMat4("u_Model", entity->getGlobalTransformMatrix());
                modelSet.mesh.draw();
            }
        }
    }
}


unsigned Renderer::getFinalSceneTexture()
{
    return hdrColorTexs[0];
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


    renderShadowMap();



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

    renderScene(cam, m_GBuffer.frameBufferHolder.getFrameBuffer(), renderWidth, renderHeight);
}
