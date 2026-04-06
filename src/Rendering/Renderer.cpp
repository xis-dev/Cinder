#include "includes/Rendering/Renderer.h"

#include "AssetManager.h"
#include "Camera.h"
#include "includes/Resources/Texture.h"
#include "includes/Primitives/Skybox.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "gtx/norm.hpp"


void Renderer::init(GLFWwindow* win, AssetManager* manager, Scene* scene)
{
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

    shadowTex = Texture::createDepthMap(3840, 2160);
    shadowFBO = create2DShadowFBO(shadowTex);

    pointShadowTex = Texture::createDepthCubemap(2048, 2048);
    pointShadowFBO = createCubemapShadowFBO(pointShadowTex);
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

    glm::vec3 lightPos = light->getPosition();
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




void Renderer::render(const Camera &cam)
{
    glEnable(GL_DEPTH_TEST);

    // Tell opengl what color we want glClear to clear the color buffer
/*    glClearColor(0.15f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);*/	 // Clear color and depth buffers
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);

    glViewport(0, 0, 3840,  2160);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glClear(GL_DEPTH_BUFFER_BIT);



    if (drawWireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (cullBackface)
    {
        glEnable(GL_CULL_FACE);
    }
    else
    {
        glDisable(GL_CULL_FACE);
    }




    glCullFace(GL_FRONT);
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
            shadowMapShader->setUniformMat4("u_Model", entity->getTransformMatrix());
            modelSet.mesh.draw();
        }
    }
    
    glViewport(0, 0, 2048, 2048);
    glBindFramebuffer(GL_FRAMEBUFFER, pointShadowFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    auto* pointMapShader = ASSET_MANAGER->shaders.get("pointMap");
    pointMapShader->use();
    for (auto* light: CURRENT_SCENE->m_lights)
    {
	    if (auto* pointLight = dynamic_cast<PointLight*>(light))
	    {
            setupPointMatrices(pointLight, 2048, 2048);
            pointMapShader->setUniformVec3("u_LightPos", pointLight->getPosition());
            pointMapShader->setUniformf("u_FarPlane", farPlane);
            for (int i = 0; i < 6; ++i)
            {
                pointMapShader->setUniformMat4(("u_ShadowMatrices[" + std::to_string(i) + "]").c_str(), shadowTransforms[i]);
            }
	    }
    }

    for (const auto& entity : CURRENT_SCENE->m_meshEnts)
    {
        for (auto& modelSet : entity->getModel()->getMeshes())
        {
            pointMapShader->setUniformMat4("u_Model", entity->getTransformMatrix());
            modelSet.mesh.draw();
        }
    }
    glViewport(0, 0, 1600, 900);


    glCullFace(GL_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    glm::vec3 camPosition = cam.getPosition();
    glm::mat4 projectionMat = cam.getProjectionMatrix();
    glm::mat4 viewMat = cam.getViewMatrix();
    glm::mat4 vpMat = projectionMat * viewMat;

    for (const auto& entity : CURRENT_SCENE->m_meshEnts)
    {
        for (auto& modelSet : entity->getModel()->getMeshes())
        {
           Material* mat = ASSET_MANAGER->materials.get(modelSet.mat);
           Shader* shader = ASSET_MANAGER->shaders.get(mat->getShader());
           shader->use();
           CURRENT_SCENE->illuminate(*shader);
           CURRENT_SCENE->applyLightCountsToShader(*shader);


           for (const auto& projView : CURRENT_SCENE->dirLightTransforms)
           {
               shader->setUniformMat4("u_LightSpaceMatrix", projView);
           }

           shader->setUniformVec3("u_CameraPosition", camPosition);
           shader->setUniformMat4("u_ProjectionMatrix", projectionMat);
           shader->setUniformMat4("u_ModelMatrix", entity->getTransformMatrix());
           shader->setUniformMat4("u_ViewMatrix", viewMat);
           shader->setUniformMat4("u_VPMatrix", vpMat);
           shader->setUniformi("u_Blinn", blinnLighting);
           shader->setUniformf("u_Gamma", gamma);

           shader->setUniformf("u_FarPlane", farPlane);

           shader->setUniformVec3("u_ViewDirection", cam.getDirection());
           shader->setUniformMat4("u_MVPMatrix", vpMat * entity->getTransformMatrix());

           const auto textures = mat->getTextures();
           int textureUnit = 0;
           int diffuseNr = 0;
           int specularNr = 0;
           int normalNr = 0;
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
                       uniformStr = "t_Normal";
                       ++normalNr;
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


           glActiveTexture(GL_TEXTURE0 + textureUnit);
           glBindTexture(GL_TEXTURE_2D, shadowTex);
           shader->setUniformi("u_ShadowMap", textureUnit);

           ++textureUnit;

           glActiveTexture(GL_TEXTURE0 + textureUnit);
           glBindTexture(GL_TEXTURE_CUBE_MAP, pointShadowTex);
           shader->setUniformi("u_PointMap", textureUnit);



           std::string materialUniformBase = "u_Material.";
           shader->setUniformVec3((materialUniformBase + "albedo").c_str(), mat->getColor());
           shader->setUniformf((materialUniformBase + "ambient").c_str(), mat->getAmbience());
           shader->setUniformf((materialUniformBase + "diffuse").c_str(), mat->getDiffuse());
           shader->setUniformf((materialUniformBase + "specular").c_str(), mat->getSpecular());
           shader->setUniformf((materialUniformBase + "shininess").c_str(), mat->getShininess());

           modelSet.mesh.draw();

            for (int i = 0; i <= textureUnit; ++i)
            {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, 0);
                glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
            }
        }
        }

 
 
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

                iconShader->setUniformVec3("u_ObjectPosition", (glm::vec3)entity->getPosition());
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

  
}
