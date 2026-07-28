#pragma once

#include "ResourceManager.h"
#include "Scene.h"

#include "GLFW/glfw3.h"
#include "glm/mat4x4.hpp"

#include <vector>
#include <string>

#include "GBuffer.h"
#include "Techniques/DeferredLightPass.h"
#include "Techniques/SSAORenderPass.h"

struct AssetManager;
class Camera;
class Texture;
class PointLight;

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

    unsigned hdrFBO, hdrDepthStencil;
    unsigned bloomFBO, bloomColor;
    unsigned hdrColorTexs[2];
    unsigned pingPongFBOs[2];
    unsigned pingPongColorBuffers[2];
    unsigned skyBoxVAO, skyBoxVBO, skyBoxEBO;
    unsigned shadowFBO, shadowTex;
    unsigned pointShadowFBO, pointShadowTex;

    GBuffer m_GBuffer;
    SSAORenderPass m_SSAOPass;
    DeferredLightPass m_LightPass;

    int renderWidth{};
    int renderHeight{};

    float farPlane{};

    bool drawWireframe{};
    bool cullBackface{true};
    bool blinnLighting{true};
    bool cubeMapEnabled{ true };
    bool drawGrid{ true };
    bool hdr{ true };
    bool fbo1{};

    std::vector<glm::mat4> shadowTransforms{};

    float gamma{ 2.2f };
    bool useSSAO{true};
    float parallaxScale{ 0.2f };
    float hdrExposure{ 1.0f };
    bool bloom{true};
private:
    void createSkybox();
    void drawSkybox(const Camera& cam);
    unsigned createFBO(unsigned *colorTexts, int colorTexCount, unsigned depthStencil = 0);
    void createPingPongFBOs();
    unsigned create2DShadowFBO(unsigned depthTex);

    static unsigned createCubemapShadowFBO(unsigned depthCubemap);
    void setupPointMatrices(PointLight* light, const int w, const int h);
    void renderScene(const Camera &cam, unsigned fboToRenderTo, int sceneW, int sceneH);
    void renderShadowMap();
    void renderPointMap(Scene *currentScene);




    void updateRenderComponents(int w, int h);
public:
    void init(GLFWwindow *win, AssetManager *manager, Scene *scene, int width, int height);

    void render(const Camera& cam);

    unsigned getFinalSceneTexture();

    void changeViewportSize(int w, int h);

    void destroy();



};
