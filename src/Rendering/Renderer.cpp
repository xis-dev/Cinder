#include "includes/Rendering/Renderer.h"

#include "AssetManager.h"
#include "Camera.h"
#include "includes/Resources/Texture.h"
#include "includes/Primitives/Skybox.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "gtx/norm.hpp"


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

unsigned Renderer::createShadowFBO(unsigned depthTex)
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


void Renderer::init(GLFWwindow *win, AssetManager *manager, Scene* scene)
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
    shadowFBO = createShadowFBO(shadowTex);
}

void Renderer::render(const Camera &cam)
{
    glEnable(GL_DEPTH_TEST);

    // Tell opengl what color we want glClear to clear the color buffer
/*    glClearColor(0.15f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);*/	 // Clear color and depth buffers

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


    glCullFace(GL_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, 1600, 900);

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
           shadowMapShader->setUniformMat4("u_Model", entity->getTransformMatrix());
           shader->setUniformMat4("u_ViewMatrix", viewMat);
           shader->setUniformMat4("u_VPMatrix", vpMat);
           shader->setUniformi("u_Blinn", blinnLighting);
           shader->setUniformf("u_Gamma", gamma);


           shader->setUniformVec3("u_ViewDirection", cam.getDirection());
           shader->setUniformMat4("u_MVPMatrix", vpMat * entity->getTransformMatrix());

           const auto textures = mat->getTextures();
           int textureUnit = 0;

           if (!textures.empty())
           {
               int diffuseNr = 0;
               int specularNr = 0;

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

               shader->setUniformi("u_DiffuseMapCount", diffuseNr);
               shader->setUniformi("u_SpecularMapCount", specularNr);
           }


           glActiveTexture(GL_TEXTURE0 + textureUnit);
           glBindTexture(GL_TEXTURE_2D, shadowTex);
           shader->setUniformi("u_ShadowMap", textureUnit);
           glActiveTexture(GL_TEXTURE0);


           std::string materialUniformBase = "u_Material.";
           shader->setUniformVec3((materialUniformBase + "albedo").c_str(), mat->getColor());
           shader->setUniformf((materialUniformBase + "ambient").c_str(), mat->getAmbience());
           shader->setUniformf((materialUniformBase + "diffuse").c_str(), mat->getDiffuse());
           shader->setUniformf((materialUniformBase + "specular").c_str(), mat->getSpecular());
           shader->setUniformf((materialUniformBase + "shininess").c_str(), mat->getShininess());

           modelSet.mesh.draw();
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

    glDepthFunc(GL_LESS);


}
