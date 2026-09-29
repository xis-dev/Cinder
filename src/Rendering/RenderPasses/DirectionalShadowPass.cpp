#include "DirectionalShadowPass.h"

#include "Scene.h"
#include "Texture.h"
#include "AABB.h"

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

    // Casters infront of near still write depth
    glEnable(GL_DEPTH_CLAMP);
    m_shader->use();

    updateSplits(m_cascadeSplits, cam.m_nearPlane, cam.m_farPlane, correctionStrength);
    const glm::vec3& lightDir = glm::normalize(scene->m_directionalLights[0]->m_direction);

    // TODO: Should actually be the AABB of all entiies/scene but dont have AABB setup yet
    // TODO: Position is wrong, opengl looks down z axis, take dot with each default basis?
    auto overallFrustumCorners = Camera::getFrustumCorners(cam.getPosition(), glm::normalize(cam.m_direction),
                                                    cam.m_nearPlane, cam.m_farPlane, cam.m_currentFov, cam.m_aspectRatio);

    AABB cameraFrustumBB = AABB::getAABB(overallFrustumCorners);
    // TODO: Should actually be the AABB of all entiies/scene but dont have AABB setup yet
    glm::vec3 lightPos = -lightDir;

    // TODO: Not sure actually, theres only 1 directional light, but need to have somewhere clipping it to one, maybe in scene
    glm::vec3 up = std::fabs(glm::dot({0.0f, 1.0f, 0.0f}, lightDir)) > 0.9999f ? glm::vec3{0.0f, 0.0f, 1.0f} : glm::vec3{0.0f, 1.0f, 0.0f};


    for (int i = 0; i < m_cascadeSplits.size(); ++i)
    {
        float near = i == 0 ? cam.m_nearPlane : m_cascadeSplits[i - 1];
        std::vector<glm::vec3> frustumCorners = Camera::getFrustumCorners(cam.getPosition(), glm::normalize(cam.m_direction),
                                                                          near, m_cascadeSplits[i], cam.m_currentFov, cam.m_aspectRatio);
        glm::vec3 center{0.0f};
        for (const auto& c: frustumCorners) {
            center += c;
        }
        center /= (float)frustumCorners.size();

        glm::mat4 view = glm::lookAt(center - lightDir, center, up);

        std::vector<glm::vec3> ls;
        ls.reserve(8);

        for (const auto& c: frustumCorners) {
            ls.emplace_back(view * glm::vec4(c, 1.0f));
        }
        auto bb = AABB::getAABB(ls);

        float texelWorld = (bb.boundsMax.x - bb.boundsMin.x) / float(mainResolution.x); // after computing ortho bounds
        m_cascadeTexelWorld[i] = texelWorld;
        // Light view looks down z, closer means larger z
        // Pad near so casters out of camera frustum but in light frustum are still included
        // TODO: Replace padding with scene AABB extent
        float casterPad = 500.0f;
        glm::mat4 proj = glm::ortho(bb.boundsMin.x, bb.boundsMax.x, bb.boundsMin.y, bb.boundsMax.y, -bb.boundsMax.z - casterPad, -bb.boundsMin.z);

        m_lightTransforms[i] = proj * view;
        m_shader->setUniformMat4("u_LightProjView", m_lightTransforms[i]);

        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texArray, 0, i);

        glClear(GL_DEPTH_BUFFER_BIT);
        for (const auto& ent: scene->m_meshEnts)
        {
            for (auto& modelSet: ent->getModel()->getMeshes())
            {
                m_shader->setUniformMat4("u_Model", ent->getGlobalTransformMatrix());
                modelSet.mesh.draw();
            }
        }

        continue;



    }

}

void DirectionalShadowPass::updatePassSize(int w, int h)
{
}

unsigned DirectionalShadowPass::getOutput(int index) const
{
    return texArray;
}
