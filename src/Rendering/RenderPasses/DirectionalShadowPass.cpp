//
// Created by PC on 29-Jul-26.
//

#include "DirectionalShadowPass.h"

#include "Quad.h"
#include "Scene.h"
#include "Texture.h"

void DirectionalShadowPass::updateSplits(std::vector<float> &splits, float near, float far, float linearCorrection)
{
    for (int i = 0; i < splits.size(); ++i)
    {
        splits[i] = (float)std::pow(linearCorrection * near * (far / near), (float)(i + 1)/ splits.size()) +
                    (1 - linearCorrection) * (near + ((float)(i + 1)/splits.size()) * (far - near));
    }
}


DirectionalShadowPass::DirectionalShadowPass(int w, int h, Shader *shader): m_shader(shader)
{
    if (!m_shader)
    {
        std::cout << "Shadow Pass, null shader.\n";
        return;
    }

    m_buffer = FrameBuffer(std::vector<unsigned>{});

    unsigned depth = Texture::createEmptyTex(w, h, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT);

    // Not actually needed since im using texture layers but maybe for completeness?
    m_buffer.attachDepthBuffer(depth, true, GL_DEPTH_ATTACHMENT);

    texArray = Texture::createEmptyTexArray(numberOfSplits, (int)mainResolution.x, (int)mainResolution.y, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT);

    glBindTexture(GL_TEXTURE_2D_ARRAY, texArray);

    for (int i = 0; i < numberOfSplits; ++i)
    {
        cascadeUniformStrings.push_back("u_CascadeDistances[" + std::to_string(i) + "]");
        m_cascadeSplits.push_back(0.0f);
      //  glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, (int)mainResolution.x, (int)mainResolution.y, 1, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    }


    enable();
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void DirectionalShadowPass::configuredRender(const FrameContext &ctx, const Camera & cam, Scene* scene)
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_buffer.getFrameBuffer());
    glViewport(0, 0, (int)mainResolution.x, (int)mainResolution.y);
    glClear(GL_DEPTH_BUFFER_BIT);

    m_shader->use();



    updateSplits(m_cascadeSplits, cam.m_nearPlane, cam.m_farPlane, correctionStrength);

    const glm::vec3& lightDir = glm::normalize(scene->m_directionalLights[0]->m_direction);

    // TODO: Should actually be the AABB of all entiies/scene but dont have AABB setup yet
    // TODO: Position is wrong, opengl looks down z axis, take dot with each default basis?
    std::vector<glm::vec3> overallFrustumCorners = Camera::getFrustumCorners(cam.getPosition(), glm::normalize(cam.m_direction),
                                                    cam.m_nearPlane, cam.m_farPlane, cam.m_currentFov, cam.m_aspectRatio);

    AABB cameraFrustumBB = AABB::getAABB(overallFrustumCorners);
    // TODO: Should actually be the AABB of all entiies/scene but dont have AABB setup yet
    glm::vec3 lightPos = (cameraFrustumBB.diagonalDistance / 2.0f) * -lightDir;

    glm::vec3 up{0.0f, 1.0f, 0.0f};

    // TODO: Not sure actually, theres only 1 directional light, but need to have somewhere clipping it to one, maybe in scene
    if (std::abs(glm::dot(up, lightDir)) > 0.999f)
    {
        up = {0.0f, 0.0f, 1.0f};
    }

    glm::vec3 basisUp = glm::normalize(up - glm::dot(up, lightDir) * lightDir);
    glm::vec3 basisRight = glm::cross(lightDir, up);

    glm::mat4 orthogonalMat = glm::mat4{glm::vec4{basisRight, 1.0f}, glm::vec4{basisUp, 1.0f}, glm::vec4{lightDir, 1.0f}, glm::vec4{0.0f, 0.0f, 0.0f, 1.0f}};

    orthogonalMat = orthogonalMat * glm::transpose(orthogonalMat);

    const glm::mat4 lightView = glm::lookAt(lightPos, cameraFrustumBB.center, up);



    for (int i = 0; i < m_cascadeSplits.size(); ++i)
    {
        float near = i < 1 ? cam.m_nearPlane : m_cascadeSplits[i - 1];
        std::vector<glm::vec3> frustumCorners = Camera::getFrustumCorners(cam.getPosition(), glm::normalize(cam.m_direction),
                                                                          near, m_cascadeSplits[i], cam.m_currentFov, cam.m_aspectRatio);

    glm::mat4 lightProjection = glm::identity<glm::mat4>();
    glm::mat4 lVP = orthogonalMat * lightView;

        std::vector<glm::vec3> frustumCorners_LightSpace{};
        frustumCorners_LightSpace.reserve(8);
        for (auto& corner: frustumCorners)
        {
            frustumCorners_LightSpace.emplace_back(lightView * glm::vec4{corner, 1.0f});

        }

    AABB lBB = AABB::getAABB(frustumCorners_LightSpace);

    // Calculate scale and offset for crop matrix
    glm::vec2 s = {2.0f / (lBB.boundsMax.x - lBB.boundsMin.x), 2.0f / (lBB.boundsMax.y - lBB.boundsMin.y)};
    glm::vec2 o = {-0.5f  * (lBB.boundsMax.x + lBB.boundsMin.x) * s.x, -0.5f * (lBB.boundsMax.y + lBB.boundsMin.y) * s.y};


    // TODO: Change vector order if im wrong about major order
    glm::mat4 cropMatrix = {glm::vec4{s.x, 0.0f, 0.0f, 0.0f},
                            glm::vec4{0.0f, s.y, 0.0f, 0.0f},
                            glm::vec4{0.0f, 0.0f, 1.0f, 0.0f},
                            glm::vec4{o.x, o.y, 0.0f, 1.0f}};

    glm::mat4 zLightProjection = glm::ortho(lBB.boundsMin.x, lBB.boundsMax.x, lBB.boundsMin.y, lBB.boundsMax.y, lBB.boundsMin.z, lBB.boundsMax.z);
    //zLightProjection = cropMatrix * zLightProjection;
       // lightProjection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 2000.0f);
       // lightProjection = glm::perspective(cam.m_currentFov, cam.m_aspectRatio, 0.1f, 2000.0f);

    m_shader->setUniformMat4("u_LightProjView", zLightProjection * cropMatrix * lightView);
    lightTransform = zLightProjection * cropMatrix * lightView;

        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texArray, 0, i);
        for (const auto& ent: scene->m_meshEnts)
        {
            for (auto& modelSet: ent->getModel()->getMeshes())
            {
                m_shader->setUniformMat4("u_Model", ent->getGlobalTransformMatrix());
                modelSet.mesh.draw();
            }
        }

        glClear(GL_DEPTH_BUFFER_BIT);

    }

}

void DirectionalShadowPass::updatePassSize(int w, int h)
{
}

unsigned DirectionalShadowPass::getOutput(int index) const
{
    return texArray;
}
