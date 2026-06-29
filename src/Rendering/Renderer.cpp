#include "includes/Rendering/Renderer.h"

#include "AssetManager.h"
#include "Camera.h"
#include "includes/Resources/Texture.h"
#include "includes/Primitives/Skybox.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <random>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "gtc/type_ptr.hpp"
#include "gtx/norm.hpp"

void Renderer::init(GLFWwindow *win, AssetManager *manager, Scene *scene, int *width, int *height)
{
    currentWindowWidth = width;
    currentWindowHeight = height;
    // TODO: Add static assertions
    WINDOW = win;
    ASSET_MANAGER = manager;
    CURRENT_SCENE = scene;

    glCullFace(GL_BACK);

    // Initialize addon VAO
    glGenVertexArrays(1, &addonVAO);
    glBindVertexArray(addonVAO);
    glBindVertexArray(0);

    createSkybox();
    cubeMapTex = Texture::createCubemap(textures_faces);

    shadowTex = Texture::createEmptyTex(1920, 1080, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT);
    shadowFBO = create2DShadowFBO(shadowTex);



    pointShadowTex = Texture::createEmptyCubemap(2048, 2048, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT);
    pointShadowFBO = createCubemapShadowFBO(pointShadowTex);

    createPingPongFBOs();
    hdrColorTexs[0] = Texture::createEmptyTex(*currentWindowWidth, *currentWindowHeight, GL_RGBA16F, GL_RGBA);
    hdrColorTexs[1] = Texture::createEmptyTex(*currentWindowWidth, *currentWindowHeight, GL_RGBA16F, GL_RGBA);
    hdrDepthStencil = Texture::createEmptyTex(*currentWindowWidth, *currentWindowHeight, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8);
    hdrFBO = createFBO(hdrColorTexs, hdrDepthStencil);

    initGBuffer(gBuffer, gPosition, gColorSpec, gNormal, *currentWindowWidth, *currentWindowHeight);


    initSSAO(ssaoFBO, ssaoColor, ssaoDepth);

    ssaoKernel = getSsaoKernel();

    glGenFramebuffers(1, &deferredFbo);
    glGenTextures(1, &deferredColor);

    glBindFramebuffer(GL_FRAMEBUFFER, deferredFbo);

    glBindTexture(GL_TEXTURE_2D, deferredColor);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, *currentWindowWidth, *currentWindowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, deferredColor, 0);

    glGenRenderbuffers(1, &deferredDepthStencil);
    glBindRenderbuffer(GL_RENDERBUFFER, deferredDepthStencil);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, *currentWindowWidth, *currentWindowHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, deferredDepthStencil);


    for (int i = 0; i < 6; ++i) shadowMatNames[i] = "u_ShadowMatrices[" + std::to_string(i) + "]";

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Incomplete complete Deferred Framebuffer. \n";
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void Renderer::initGBuffer(unsigned &framebuffer, unsigned &position, unsigned &colorSpec, unsigned &normal, const int w, const int h)
{
    glGenFramebuffers(1, &framebuffer);
    glGenTextures(1, &position);
    glGenTextures(1, &colorSpec);
    glGenTextures(1, &normal);
    glGenTextures(1, &gMaterial);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glBindTexture(GL_TEXTURE_2D, position);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, position, 0);

    glBindTexture(GL_TEXTURE_2D, colorSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, colorSpec, 0);

    glBindTexture(GL_TEXTURE_2D, normal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, normal, 0);

    glBindTexture(GL_TEXTURE_2D, gMaterial);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gMaterial, 0);


    GLenum attachments[4] {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
    glDrawBuffers(4, attachments);

    glGenRenderbuffers(1, &gDepthStencil);
    glBindRenderbuffer(GL_RENDERBUFFER, gDepthStencil);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, *currentWindowWidth, *currentWindowHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, gDepthStencil);


    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Incomplete complete GBuffer Framebuffer. \n";
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::initSSAO(unsigned &fb, unsigned &c, unsigned &d_st)
{
    glGenFramebuffers(1, &fb);
    glGenTextures(1, &c);

    glBindFramebuffer(GL_FRAMEBUFFER, fb);

    glBindTexture(GL_TEXTURE_2D, c);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, *currentWindowWidth, *currentWindowHeight, 0, GL_RED, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, c, 0);



    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Incomplete complete SSAO Framebuffer. \n";
    }

    std::vector<glm::vec3> ssaoNoiseCont;
    std::uniform_real_distribution<float> randomFloats(0.0, 1.0);
    std::default_random_engine generator;
    for (unsigned int i = 0; i < 16; i++)
    {
        glm::vec3 noise(
            randomFloats(generator) * 2.0 - 1.0,
            randomFloats(generator) * 2.0 - 1.0,
            0.0f);
        ssaoNoiseCont.push_back(noise);
    }
    glGenTextures(1, &ssaoNoise);
    glBindTexture(GL_TEXTURE_2D, ssaoNoise);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, ssaoNoiseCont.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);

    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &ssaoBlurFBO);
    glGenTextures(1, &ssaoBlurColor);
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);

    glBindTexture(GL_TEXTURE_2D, ssaoBlurColor);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, *currentWindowWidth, *currentWindowHeight, 0, GL_RED, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoBlurColor, 0);


    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Incomplete complete SSAO Blur Framebuffer. \n";
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void Renderer::drawAddon(int indexCount)
{
    glBindVertexArray(addonVAO);
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)indexCount);
    glBindVertexArray(0);
}

void Renderer::createSkybox()
{
    ASSET_MANAGER->shaders.add(Shader{ "assets/Shaders/skybox/skybox.vert", "assets/Shaders/skybox/skybox.frag" }, "skybox");
    glGenBuffers(1, &skyBoxVBO);
    glGenVertexArrays(1, &skyBoxVAO);
    glBindVertexArray(skyBoxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyBoxVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(SkyBox::vertices), SkyBox::vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}


void Renderer::drawSkybox(const Camera& cam)
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

unsigned Renderer::createFBO(unsigned* colorTexts, unsigned depthStencil)
{
    std::vector<unsigned> attachments{};
    unsigned id;
    glGenFramebuffers(1, &id);
    glBindFramebuffer(GL_FRAMEBUFFER, id);
    
    for (int i = 0; i < 2; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, colorTexts[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorTexts[i], 0);
        attachments.push_back(GL_COLOR_ATTACHMENT0 + i);

    }
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthStencil, 0);


    glDrawBuffers(attachments.size(), attachments.data());
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Incomplete complete HDR Framebuffer. \n";
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return id;
}

void Renderer::createPingPongFBOs()
{
    glGenFramebuffers(2, pingPongFBOs);
    glGenTextures(2, pingPongColorBuffers);

    for (int i = 0; i < 2; ++i)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingPongFBOs[i]);
        glBindTexture(GL_TEXTURE_2D, pingPongColorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, *currentWindowWidth, *currentWindowHeight, 0, GL_RGBA, GL_FLOAT, nullptr);

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
    }
    else
    {
        std::cout << "Complete: " << GL_FRAMEBUFFER_COMPLETE << std::endl;
        std::cout << "InCompleteaTT: " << GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT << std::endl;
        std::cout << "Complete: " << GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return id;
}

void Renderer::setupPointMatrices(PointLight* light, const int w, const int h)
{
    shadowTransforms.clear();
    float aspect = (float)w / (float)h;
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

void Renderer::renderScene(const Camera& cam, unsigned fboToRenderTo, const int sceneW, const int sceneH)
{
    // Disable blending for deferred
    glDisable(GL_BLEND);
    glBindFramebuffer(GL_FRAMEBUFFER, fboToRenderTo);
    glViewport(0, 0, sceneW, sceneH);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    glm::vec3 camPosition = cam.getPosition();
    glm::mat4 projectionMat = cam.getProjectionMatrix();
    glm::mat4 viewMat = cam.getViewMatrix();
    glm::mat4 vpMat = projectionMat * viewMat;


    for (Shader* shader : CURRENT_SCENE->m_renderBatches | std::views::keys)
    {
        for (const auto& [modelSet, entity]: CURRENT_SCENE->m_renderBatches[shader])
        {
            Material* mat = ASSET_MANAGER->materials.get(modelSet->mat);
            shader->use();

            shader->setUniformVec3("u_CameraPosition", camPosition);
            shader->setUniformMat4("u_ProjectionMatrix", projectionMat);
            shader->setUniformMat4("u_ModelMatrix", entity->getGlobalTransformMatrix());
            shader->setUniformMat4("u_ViewMatrix", viewMat);
            shader->setUniformMat4("u_VPMatrix", vpMat);
            shader->setUniformf("u_ParallaxHeightScale", parallaxScale);

            shader->setUniformVec3("u_ViewDirection", cam.getDirection());
            shader->setUniformMat4("u_MVPMatrix", vpMat * entity->getGlobalTransformMatrix());


            const auto textures = mat->getTextures();
            int textureUnit = 0;
            int diffuseNr = 0;
            int specularNr = 0;
            int normalNr = 0;
            int heightNr = 0;
            if (!textures.empty())
            {
                for (auto texture : textures)
                {
                    auto* currentTex = ASSET_MANAGER->textures.get(texture);
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

    if (useSSAO)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
    glDisable(GL_CULL_FACE);
        glClear(GL_COLOR_BUFFER_BIT);

        auto* ssaoShader = ASSET_MANAGER->shaders.get("ssaoShader");
        if (!ssaoShader)
        {
            std::cout << "SSAO Shader null.\n";
            return;
        }
        ssaoShader->use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPosition);
        ssaoShader->setUniformi("u_GPosition", 0);


        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gColorSpec);
        ssaoShader->setUniformi("u_GColorSpec", 1);


        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        ssaoShader->setUniformi("u_GNormal", 2);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, ssaoNoise);
        ssaoShader->setUniformi("t_Noise", 3);

        ssaoShader->setUniformf("u_SsaoPow", ssaoStr);
        ssaoKernel = getSsaoKernel();
        ssaoShader->setUniformVec3Array("samples", ssaoKernel.data(), 64);
        ssaoShader->setUniformMat4("u_ViewMatrix", viewMat);
        ssaoShader->setUniformMat4("u_Projection", projectionMat);
        drawAddon(6);

        glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
        glClear(GL_COLOR_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, ssaoColor);

        auto* ssaoBlurShader = ASSET_MANAGER->shaders.get("ssaoBlur");

        ssaoBlurShader->use();
        ssaoBlurShader->setUniformi("t_SsaoInput", 3);

        drawAddon(6);


    }

    glBindFramebuffer(GL_FRAMEBUFFER, deferredFbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_CULL_FACE);
    auto* lightPassShader = ASSET_MANAGER->shaders.get("deferredLightPass");
    lightPassShader->use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    lightPassShader->setUniformi("u_GPosition", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gColorSpec);
    lightPassShader->setUniformi("u_GColorSpec", 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    lightPassShader->setUniformi("u_GNormal", 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, gMaterial);
    lightPassShader->setUniformi("u_GMaterial", 3);

    lightPassShader->setUniformi("u_SSAOActive", useSSAO);
    if (useSSAO)
    {
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, ssaoBlurColor);
        lightPassShader->setUniformi("u_SSAO", 4);
    }


    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, shadowTex);
    lightPassShader->setUniformi("u_ShadowMap", 5);

    int pointMapStartIdx = 6;
    int loopIdx = 0;
    for (auto &[shadowCubemap, shadowMapTransforms]: CURRENT_SCENE->m_pointShadows | std::views::values)
    {
        glActiveTexture(GL_TEXTURE0 + pointMapStartIdx + loopIdx);
        glBindTexture(GL_TEXTURE_CUBE_MAP, shadowCubemap);
        lightPassShader->setUniformi(("t_PointMaps[" + std::to_string(loopIdx) + "]").c_str(), pointMapStartIdx + loopIdx);
        ++loopIdx;
    }

    CURRENT_SCENE->illuminate(*lightPassShader);
    CURRENT_SCENE->applyLightCountsToShader(*lightPassShader);
    for (const auto& projView : CURRENT_SCENE->dirLightTransforms)
    {
        lightPassShader->setUniformMat4("u_LightSpaceMatrix", projView);
    }

    lightPassShader->setUniformVec3("u_CameraPosition", camPosition);
    lightPassShader->setUniformMat4("u_ProjectionMatrix", projectionMat);
    lightPassShader->setUniformVec3("u_ViewDirection", cam.getDirection());
    lightPassShader->setUniformi("u_Blinn", blinnLighting);
    lightPassShader->setUniformf("u_Gamma", gamma);
    lightPassShader->setUniformf("u_FarPlane", farPlane);
    lightPassShader->setUniformf("u_HDRExposure", hdrExposure);

    lightPassShader->setUniformMat4("u_ViewMatrix", viewMat);
    drawAddon(6);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);

    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, deferredFbo);
    glBlitFramebuffer(0, 0, *currentWindowWidth, *currentWindowHeight, 0, 0, *currentWindowWidth, *currentWindowHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, deferredFbo);
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
        if (auto* gridShader = ASSET_MANAGER->shaders.get("grid"))
        {
            gridShader->use();
            gridShader->setUniformMat4("u_VPMatrix", (cam.getProjectionMatrix() * cam.getViewMatrix()));
            gridShader->setUniformVec3("u_CameraPosition", cam.getPosition());
            gridShader->setUniformf("u_Gamma", gamma);

            drawAddon(6);
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

        glm::vec3 cameraRightWorldSpace = glm::vec3{ view[0][0], view[1][0], view[2][0] };
        glm::vec3 cameraUpWorldSpace = glm::vec3{ view[0][1], view[1][1], view[2][1] };
        iconShader->setUniformVec3("u_CameraRight_WorldSpace", cameraRightWorldSpace);

        iconShader->setUniformVec3("u_CameraUp_WorldSpace", cameraUpWorldSpace);

        for (auto& entity : CURRENT_SCENE->getEntities())
        {
            if (entity->hasIcon())
            {
                iconShader->use();

                iconShader->setUniformVec3("u_ObjectPosition", (glm::vec3)entity->getWorldPosition());
                iconShader->setUniformMat4("u_ProjectionMatrix", projection);
                iconShader->setUniformMat4("u_ViewMatrix", view);
                iconShader->setUniformf("u_Gamma", gamma);

                glActiveTexture(GL_TEXTURE0);
                entity->tryGetIcon()->use();
                iconShader->setUniformi("u_iconImage", 0);
                drawAddon(6);
            }
        }
    }

    glDisable(GL_CULL_FACE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, deferredColor);

    auto* hdrShader = ASSET_MANAGER->shaders.get("HDR");
    hdrShader->use();
    hdrShader->setUniformi("u_HDRTexture", 0);
    hdrShader->setUniformf("u_HDRExposure", hdrExposure);
    hdrShader->setUniformf("u_Gamma", gamma);

    drawAddon(6);
    glEnable(GL_CULL_FACE);





}

void Renderer::renderShadowMap()
{
    glViewport(0, 0, 1920, 1080);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    auto* shadowMapShader = ASSET_MANAGER->shaders.get("shadowMap");
    if (shadowMapShader)
    {
        shadowMapShader->use();
        for (const auto& projView : CURRENT_SCENE->dirLightTransforms)
        {
            shadowMapShader->setUniformMat4("u_LightProjView", projView);
        }

    }

    for (const auto& entity : CURRENT_SCENE->m_meshEnts)
    {
        for (auto& modelSet : entity->getModel()->getMeshes())
        {
            shadowMapShader->setUniformMat4("u_Model", entity->getGlobalTransformMatrix());
            modelSet.mesh.draw();
        }
    }
}

void Renderer::renderPointMap(Scene* currentScene)
{
    glBindFramebuffer(GL_FRAMEBUFFER, pointShadowFBO);
    auto* pointMapShader = ASSET_MANAGER->shaders.get("pointMap");
    pointMapShader->use();
    currentScene->setupPointMatrices(2048, 2048);
    for (std::pair<PointLight*, PointShadow> ps : currentScene->m_pointShadows)
    {
        auto* light = ps.first;
        auto& shadow = ps.second;
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadow.shadowCubemap, 0);
        glClear(GL_DEPTH_BUFFER_BIT);
        pointMapShader->setUniformVec3("u_LightPos", light->getWorldPosition());
        pointMapShader->setUniformf("u_FarPlane", light->m_radius);

        for (int i = 0; i < 6; ++i)
        {
            pointMapShader->setUniformMat4(shadowMatNames[i].c_str(), shadow.shadowMapTransforms[i]);
        }

        for (const auto& entity : CURRENT_SCENE->m_meshEnts)
        {
            for (auto& modelSet : entity->getModel()->getMeshes())
            {
                pointMapShader->setUniformMat4("u_Model", entity->getGlobalTransformMatrix());
                modelSet.mesh.draw();
            }
        }
    }


}

 std::vector<glm::vec3> Renderer::getSsaoKernel()
{
    std::uniform_real_distribution<float> randomFloats(0., 1.);
    std::default_random_engine generator;
    std::vector<glm::vec3> ssaoKernel;

    for (unsigned i = 0; i < 64; ++i)
    {
        glm::vec3 sample (randomFloats(generator) * 2.0 - 1.0,
                          randomFloats(generator) * 2.0 - 1.0,
                          randomFloats(generator));
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);
        float scale = (float)i / 64.0;
        scale = std::lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        ssaoKernel.push_back(sample);
    }

    return ssaoKernel;
}


void Renderer::render(const Camera &cam)
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);

    if (drawWireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
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
    }
    else
    {
        glDisable(GL_CULL_FACE);
    }

    renderScene(cam, gBuffer, *currentWindowWidth, *currentWindowHeight);
    // if (hdr)
    // {
    //     renderScene(cam, hdrFBO, 1600, 900);
    // glDisable(GL_CULL_FACE);
    // glDisable(GL_DEPTH_TEST);
    //
    //     if (bloom)
    //     {
    //         auto* shaderBlur = ASSET_MANAGER->shaders.get("bloomBlur");
    //         auto* bloomShader = ASSET_MANAGER->shaders.get("bloom");
    //         bool horizontal = true, first_iteration = true;
    //         int amount = 10;
    //         shaderBlur->use();
    //         glActiveTexture(GL_TEXTURE0);
    //         shaderBlur->setUniformi("image", 0);
    //         for (unsigned int i = 0; i < amount; i++)
    //         {
    //             glBindFramebuffer(GL_FRAMEBUFFER, pingPongFBOs[horizontal]);
    //             shaderBlur->setUniformi("horizontal", horizontal);
    //             glBindTexture(
    //                 GL_TEXTURE_2D, first_iteration ? hdrColorTexs[1] : pingPongColorBuffers[!horizontal]
    //             );
    //             drawAddon(6);
    //             horizontal = !horizontal;
    //             if (first_iteration)
    //                 first_iteration = false;
    //         }
    //
    //         glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //         glViewport(0, 0, 1600, 900);
    //         glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //         bloomShader->use();
    //
    //         bloomShader->setUniformi("scene", 0);
    //         glActiveTexture(GL_TEXTURE0);
    //         glBindTexture(GL_TEXTURE_2D, hdrColorTexs[0]);
    //
    //         bloomShader->setUniformi("bloomBlur", 1);
    //         glActiveTexture(GL_TEXTURE1);
    //         glBindTexture(GL_TEXTURE_2D, pingPongColorBuffers[!horizontal]);
    //
    //         bloomShader->setUniformf("u_Gamma", gamma);
    //         bloomShader->setUniformf("exposure", hdrExposure);
    //
    //         drawAddon(6);
    //     }
    //     else
    //     {
    //         glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //         glViewport(0, 0, 1600, 900);
    //         auto* hdrScreen = ASSET_MANAGER->shaders.get("HDR");
    //         hdrScreen->use();
    //         glActiveTexture(GL_TEXTURE0);
    //         glBindTexture(GL_TEXTURE_2D, hdrColorTexs[0]);
    //         hdrScreen->setUniformi("u_HDRTexture", 0);
    //         hdrScreen->setUniformf("u_HDRExposure", hdrExposure);
    //         hdrScreen->setUniformf("u_Gamma", gamma);
    //         drawAddon(6);
    //     }
    //
    //
    // }
    // else
    // {
    //     //renderScene(cam, 0, *currentWindowWidth, *currentWindowHeight);
    //     glDisable(GL_BLEND);
    //     renderScene(cam, gBuffer, *currentWindowWidth, *currentWindowHeight);
    //     glEnable(GL_BLEND);

        // glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // glClearColor(0.0, 0.0, 0.0, 1.0);
        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // glDisable(GL_CULL_FACE);
        // auto* screenShader = ASSET_MANAGER->shaders.get("screenShader");
        // screenShader->use();
        //
        // glActiveTexture(GL_TEXTURE0);
        // glBindTexture(GL_TEXTURE_2D, gPosition);
        //
        // glActiveTexture(GL_TEXTURE1);
        // glBindTexture(GL_TEXTURE_2D, gColorSpec);
        //
        // glActiveTexture(GL_TEXTURE2);
        // glBindTexture(GL_TEXTURE_2D, gNormal);
        //
        // screenShader->setUniformi("u_Pos", 0);
        // screenShader->setUniformi("u_ColSpec", 1);
        // screenShader->setUniformi("u_Norm", 2);
        //
        // drawAddon(6);
        // glActiveTexture(GL_TEXTURE0);
        // glBindTexture(GL_TEXTURE_2D, 0);
        //
        // glActiveTexture(GL_TEXTURE1);
        // glBindTexture(GL_TEXTURE_2D, 0);
        //
        // glActiveTexture(GL_TEXTURE2);
        // glBindTexture(GL_TEXTURE_2D, 0);
        //
        // glEnable(GL_CULL_FACE);

    // }
    
}
