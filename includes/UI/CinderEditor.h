#pragma once
#include "Delegate.h"
#include "GLFW/glfw3.h"

class RenderConfigPanel;
class Entity;
class SceneHierarchyPanel;
class PropertiesPanel;
class RenderPanel;
class Renderer;
class Scene;
class ImGuiPanel;

class CinderEditor
{
public:
    CinderEditor() = default;
private:
    Scene* CURRENT_SCENE{nullptr};
    Renderer* RENDERER{nullptr};


    void renderMenuBar();

    void OnWindowSizeChanged(GLFWwindow* win, int w, int h);

    // Panels

    std::vector<ImGuiPanel*> m_panels; // List of panels generalized
    std::unique_ptr<RenderPanel> renderPanel{nullptr};
    std::unique_ptr<PropertiesPanel> propertiesPanel{nullptr};
    std::unique_ptr<SceneHierarchyPanel> sceneHierarchy{nullptr};
    std::unique_ptr<RenderConfigPanel> renderConfig{nullptr};

public:
public:
    Delegate<Entity*> onItemSelected;

    void initialize(GLFWwindow* window, Scene* scene, Renderer* renderer);
    void startRender();
    void endRender();
};

