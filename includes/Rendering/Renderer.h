#pragma once

#include "ResourceManager.h"
#include "Scene.h"
#include "GLFW/glfw3.h"

#include <vector>
#include <string>

struct AssetManager;
class Camera;
class Texture;

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
    
    unsigned skyBoxVAO, skyBoxVBO, skyBoxEBO;
    unsigned shadowFBO, shadowTex;

    bool drawWireframe{};
    bool cullBackface{};
    bool blinnLighting{true};
    bool cubeMapEnabled{ true };
    bool drawGrid{ true };

    float gamma{ 2.2f };
private:
    void drawAddon(int indexCount);
    void createSkybox();
    void drawSkybox(const Camera& cam);
    unsigned createShadowFBO(unsigned depthTex);
public:
    void init(GLFWwindow* win, AssetManager* manager, Scene* scene);
    void render(const Camera& cam);
    void destroy();

};
