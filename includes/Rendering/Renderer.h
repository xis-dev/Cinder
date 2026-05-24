#pragma once

#include "ResourceManager.h"
#include "Scene.h"

#include "GLFW/glfw3.h"
#include "glm/mat4x4.hpp"

#include <vector>
#include <string>

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
    unsigned cubeMapTex;
    unsigned addonVAO{};
    
    unsigned hdrFBO, hdrDepthStencil;
    unsigned hdrColorTexs[2];
    unsigned pingPongFBOs[2];
    unsigned pingPongColorBuffers[2];
    unsigned skyBoxVAO, skyBoxVBO, skyBoxEBO;
    unsigned shadowFBO, shadowTex;
    unsigned pointShadowFBO, pointShadowTex;

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
    float parallaxScale{ 0.2f };
    float hdrExposure{ 1.0f };
    bool bloom{true};
private:
    void drawAddon(int indexCount);
    void createSkybox();
    void drawSkybox(const Camera& cam);
    unsigned createFBO(unsigned* colorTexts, unsigned depthStencil);
    void createPingPongFBOs();
    unsigned create2DShadowFBO(unsigned depthTex);
    unsigned createCubemapShadowFBO(unsigned depthCubemap);
    void setupPointMatrices(PointLight* light, const int w, const int h);
    void renderScene(const Camera &cam, unsigned fboToRenderTo, int sceneW, int sceneH);
    void renderShadowMap();
    void renderPointMap();
public:
    void init(GLFWwindow* win, AssetManager* manager, Scene* scene);
    void render(const Camera& cam);
    void destroy();

};
