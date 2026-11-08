#pragma once
#include "Delegate.h"
#include "GLFW/glfw3.h"

#include <memory>

#include "KeyCodes.h"
#include "src/Core/Window.h"

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
    Window* m_capturingWindow{nullptr};


    void renderMenuBar();

    void OnWindowSizeChanged(GLFWwindow* win, int w, int h);

    // Panels

    std::vector<ImGuiPanel*> m_panels; // List of panels generalized
    std::unique_ptr<RenderPanel> renderPanel{nullptr};
    std::unique_ptr<PropertiesPanel> propertiesPanel{nullptr};
    std::unique_ptr<SceneHierarchyPanel> sceneHierarchy{nullptr};
    std::unique_ptr<RenderConfigPanel> renderConfig{nullptr};


    // Input Bindings
    ScopedBinding<Key, KeyMod> InputKeyPressedBinding;
    ScopedBinding<Key, KeyMod> InputKeyReleased;

    ScopedBinding<MouseButton, KeyMod> InputMousePressedBinding;
    ScopedBinding<MouseButton, KeyMod> InputMouseReleasedBinding;

    ScopedBinding<unsigned> InputCharBinding;

    ScopedBinding<double, double> InputCursorMovedBinding;
    ScopedBinding<double, double> InputScrollBinding;

    // Window Bindings
    ScopedBinding<int, int> WindowResizedBinding;

    ScopedBinding<uint8_t> WindowFocusBinding;
    ScopedBinding<uint8_t> WindowCursorEnterBinding;
    ScopedBinding<uint8_t> WindowMinimizedBinding;


public:
public:
    Delegate<Entity*> onItemSelected;

    void initialize(Window *window, Scene *scene, Renderer *renderer);
    void destroy();
    void beginFrame();
    void update();
    void endFrame();

private:
    // Window callbacks
    void windowFocusCallback(uint8_t focused);
    void windowCursorEnterCallback(uint8_t entered);
    void windowMinimizedCallback(uint8_t minimized);
    void windowResizedCallback(int width, int height);

    // Input callbacks
    void inputKeyPressedCallback(Key key, KeyMod mods);
    void inputKeyReleasedCallback(Key key, KeyMod mods);


    void inputMouseButtonPressed(MouseButton button, KeyMod mods);
    void inputMouseButtonReleased(MouseButton button, KeyMod mods);
    void inputCharCallback(unsigned codepoint);
    void inputCursorMovedCallback(double x, double y);
    void inputScrollCallback(double xOffset, double yOffset);
};

