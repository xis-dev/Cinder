#pragma once

#include "ResourceManager.h"
#include "Scene.h"

#include "GLFW/glfw3.h"
#include "glm/mat4x4.hpp"

#include <vector>
#include <string>

#include "GBuffer.h"
#include "Components/LightComponent.h"
#include "RenderPasses/DeferredLightPass.h"
#include "RenderPasses/SSAORenderPass.h"
#include "src/Rendering/RenderPasses/BloomPass.h"
#include "src/Rendering/RenderPasses/DirectionalShadowPass.h"
#include "src/Rendering/RenderPasses/HDRPass.h"

class BloomPass;
struct AssetManager;
class Camera;
class Texture;
class PointLight;

struct LightComponent;

class Renderer
{
public:
    Renderer() = default;
    GLFWwindow* WINDOW;
    AssetManager* ASSET_MANAGER;
    Scene* CURRENT_SCENE;
    std::vector<std::string> textures_faces{
        "assets/Textures/skybox/right.jpg",
        "assets/Textures/skybox/left.jpg",
        "assets/Textures/skybox/top.jpg",
        "assets/Textures/skybox/bottom.jpg",
        "assets/Textures/skybox/front.jpg",
        "assets/Textures/skybox/back.jpg"
        };
    std::string shadowMatNames[6];
    unsigned cubeMapTex;

    unsigned skyBoxVAO, skyBoxVBO, skyBoxEBO;
    unsigned shadowFBO, shadowTex;
    unsigned pointShadowFBO, pointShadowTex;

    GBuffer m_GBuffer;
    SSAORenderPass m_SSAOPass;
    DeferredLightPass m_LightPass;
    BloomPass m_BloomPass;
    HDRPass m_HDRPass;
    DirectionalShadowPass m_DirShadowPass;

    std::vector<RenderPass*> m_renderPasses{};

    int m_renderWidth{};
    int m_renderHeight{};

    float gamma{2.2f};
    bool drawWireframe{};
    bool cullBackface{true};
    bool cubeMapEnabled{ true };
    bool drawGrid{ true };

    std::vector<glm::mat4> shadowTransforms{};

    float parallaxScale{ 0.2f };
private:
    void createSkybox();
    void drawSkybox(const Camera& cam);
    unsigned createFBO(unsigned *colorTexts, int colorTexCount, unsigned depthStencil = 0);
    unsigned create2DShadowFBO(unsigned depthTex);

    static unsigned createCubemapShadowFBO(unsigned depthCubemap);
    void renderScene(const Camera &cam, unsigned fboToRenderTo, int sceneW, int sceneH);
    void renderShadowMap();
    void renderPointMap(Scene *currentScene);

    // TODO: Doesnt belong here, move to some light or shadow related
    static std::array<glm::mat4, 6> getPointMapMatrices(const Entity &light, int w, int h);

    // TODO: Doesnt belong here, move to some light related, really just for caching so we dont need to recreate each time
    std::unordered_map<LightComponent::Type, uint32_t> m_lightCounts{};

    void updateRenderComponents(int w, int h);
public:
    void init(GLFWwindow *win, AssetManager *manager, Scene *scene, int width, int height);

    void render(const Camera& cam);

    unsigned getFinalSceneTexture();

    void changeViewportSize(int w, int h);

    void imguiRender();

    void destroy();



};
