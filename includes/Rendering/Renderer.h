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
    std::string shadowMatNames[6];
    unsigned cubeMapTex;
    unsigned addonVAO{};

    unsigned gBuffer;
    unsigned gPosition, gColorSpec, gNormal, gDepthStencil, gMaterial;
    unsigned ssaoFBO, ssaoColor, ssaoDepth, ssaoNoise;
    unsigned ssaoBlurFBO, ssaoBlurColor;
    unsigned deferredFbo, deferredColor, deferredDepthStencil;
    unsigned hdrFBO, hdrDepthStencil;
    unsigned hdrColorTexs[2];
    unsigned pingPongFBOs[2];
    unsigned pingPongColorBuffers[2];
    unsigned skyBoxVAO, skyBoxVBO, skyBoxEBO;
    unsigned shadowFBO, shadowTex;
    unsigned pointShadowFBO, pointShadowTex;

    int* currentWindowWidth;
    int* currentWindowHeight;

    float farPlane{};

    bool drawWireframe{};
    bool cullBackface{true};
    bool blinnLighting{true};
    bool cubeMapEnabled{ true };
    bool drawGrid{ true };
    bool hdr{ true };
    bool fbo1{};

    std::vector<glm::mat4> shadowTransforms{};
    std::vector<glm::vec3> ssaoKernel;

    float gamma{ 2.2f };
    float ssaoStr{2.0f};
    bool useSSAO{true};
    float parallaxScale{ 0.2f };
    float hdrExposure{ 1.0f };
    bool bloom{true};
private:
    void initGBuffer(unsigned& framebuffer, unsigned& position, unsigned& colorSpec, unsigned& normal, const int w, const int h);
    void initSSAO(unsigned& fb, unsigned& c, unsigned& d_st);
    void drawAddon(int indexCount);
    void createSkybox();
    void drawSkybox(const Camera& cam);
    unsigned createFBO(unsigned* colorTexts, unsigned depthStencil);
    void createPingPongFBOs();
    unsigned create2DShadowFBO(unsigned depthTex);

    static unsigned createCubemapShadowFBO(unsigned depthCubemap);
    void setupPointMatrices(PointLight* light, const int w, const int h);
    void renderScene(const Camera &cam, unsigned fboToRenderTo, int sceneW, int sceneH);
    void renderShadowMap();
    void renderPointMap(Scene *currentScene);
    std::vector<glm::vec3> getSsaoKernel();
public:
    void init(GLFWwindow* win, AssetManager* manager, Scene* scene, int* width, int* height);
    void render(const Camera& cam);
    void destroy();

};
