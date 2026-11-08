#include "../../includes/UI/CinderEditor.h"

#include "Engine.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"

#include "Delegate.h"


void CinderEditor::initialize(Window* window, Scene *scene, Renderer *renderer)
{

    m_capturingWindow = window;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window->getGlfwWindow(), false);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();

    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Bind window callbacks
    WindowResizedBinding = (window->OnWindowResized.bindScoped(this, &CinderEditor::windowResizedCallback));
    WindowFocusBinding = (window->OnWindowFocused.bindScoped(this, &CinderEditor::windowFocusCallback));
    WindowCursorEnterBinding = (window->OnCursorEnter.bindScoped(this, &CinderEditor::windowCursorEnterCallback));
    WindowMinimizedBinding = (window->OnWindowMinimized.bindScoped(this, &CinderEditor::windowMinimizedCallback));

    InputKeyPressedBinding = Input::OnKeyPressed.bindScoped(this, &CinderEditor::inputKeyPressedCallback);
    InputKeyReleased = Input::OnKeyReleased.bindScoped(this, &CinderEditor::inputKeyReleasedCallback);

    InputCharBinding = Input::CharDelegate.bindScoped(this, &CinderEditor::inputCharCallback);

    InputMousePressedBinding = Input::OnMouseButtonPressed.bindScoped(this, &CinderEditor::inputMouseButtonPressed);
    InputMouseReleasedBinding = Input::OnMouseButtonReleased.bindScoped(this, &CinderEditor::inputMouseButtonReleased);

    InputCursorMovedBinding = Input::OnCursorMoved.bindScoped(this, &CinderEditor::inputCursorMovedCallback);
    InputScrollBinding = Input::OnScroll.bindScoped(this, &CinderEditor::inputScrollCallback);

    CURRENT_SCENE = scene;
    RENDERER = renderer;

    renderPanel = std::make_unique<RenderPanel>(RenderPanel("Render", RENDERER, 400, 225, (16./9)));
    m_panels.push_back(renderPanel.get());

    propertiesPanel = std::make_unique<PropertiesPanel>("Properties", onItemSelected, CURRENT_SCENE->OnEntityDestroyed);
    m_panels.push_back(propertiesPanel.get());

    sceneHierarchy = std::make_unique<SceneHierarchyPanel>("Scene Hierarchy", CURRENT_SCENE, onItemSelected);
    m_panels.push_back(sceneHierarchy.get());

    renderConfig = std::make_unique<RenderConfigPanel>("Renderer Config", RENDERER);
    m_panels.push_back(renderConfig.get());


}

void CinderEditor::destroy() {

    // ImGui shutdown
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void CinderEditor::renderMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New")){}
            if (ImGui::BeginMenu("Open", "Ctrl+O"))
            {
                ImGui::MenuItem("testfile.cpp");
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Window"))
        {
            for (auto* panel: m_panels)
            {
                if(ImGui::MenuItem(panel->getName().c_str(), nullptr, panel->canRender()))
                {
                    panel->togglePanelRendering();
                }
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}


void CinderEditor::beginFrame()
{
}

void CinderEditor::update() {

    // setup imgui for a new frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::DockSpaceOverViewport();

    renderMenuBar();

    std::vector<ImGuiPanel*> sortedPanelNames;
    int currentIdx = 0;

    // Render the panels
    for (auto& panel : m_panels)
    {
        panel->render();
    }

    // End frame
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void CinderEditor::endFrame()
{
}

void CinderEditor::windowFocusCallback(uint8_t focused) {

    if (m_capturingWindow == nullptr) {
        throw std::runtime_error("EDITOR: Capturing null pointer window.\n");
    }

    ImGui_ImplGlfw_WindowFocusCallback(m_capturingWindow->getGlfwWindow(), focused);
}

void CinderEditor::windowCursorEnterCallback(uint8_t entered) {
    if (m_capturingWindow == nullptr) {
        throw std::runtime_error("EDITOR: Capturing null pointer window.\n");
    }

    ImGui_ImplGlfw_CursorEnterCallback(m_capturingWindow->getGlfwWindow(), entered);
}

void CinderEditor::windowMinimizedCallback(uint8_t minimized) {

    if (m_capturingWindow == nullptr) {
        throw std::runtime_error("EDITOR: Capturing null pointer window.\n");
    }

}

void CinderEditor::windowResizedCallback(int width, int height) {

    if (m_capturingWindow == nullptr) {
        throw std::runtime_error("EDITOR: Capturing null pointer window.\n");
    }

}

void CinderEditor::inputKeyPressedCallback(Key key, KeyMod mods) {
    ImGui_ImplGlfw_KeyCallback(m_capturingWindow->getGlfwWindow(), static_cast<int>(key),
                               glfwGetKeyScancode(static_cast<int>(key)), GLFW_PRESS, static_cast<int>(mods.bits));
}

void CinderEditor::inputKeyReleasedCallback(Key key, KeyMod mods) {
    ImGui_ImplGlfw_KeyCallback(m_capturingWindow->getGlfwWindow(), static_cast<int>(key),
                               glfwGetKeyScancode(static_cast<int>(key)), GLFW_RELEASE, static_cast<int>(mods.bits));
}

void CinderEditor::inputMouseButtonPressed(MouseButton button, KeyMod mods) {
    ImGui_ImplGlfw_MouseButtonCallback(m_capturingWindow->getGlfwWindow(),(int)button, GLFW_PRESS, (int)mods.bits);
}

void CinderEditor::inputMouseButtonReleased(MouseButton button, KeyMod mods) {

    ImGui_ImplGlfw_MouseButtonCallback(m_capturingWindow->getGlfwWindow(),(int)button, GLFW_RELEASE, (int)mods.bits);
}

void CinderEditor::inputCharCallback(unsigned codepoint) {

    if (m_capturingWindow == nullptr) {
        throw std::runtime_error("EDITOR: Capturing null pointer window.\n");
    }

    ImGui_ImplGlfw_CharCallback(m_capturingWindow->getGlfwWindow(), codepoint);
}


void CinderEditor::inputCursorMovedCallback(double x, double y) {
    ImGui_ImplGlfw_CursorPosCallback(m_capturingWindow->getGlfwWindow(), x, y);
}

void CinderEditor::inputScrollCallback(double xOffset, double yOffset) {
    ImGui_ImplGlfw_ScrollCallback(m_capturingWindow->getGlfwWindow(), xOffset, yOffset);
}


void CinderEditor::OnWindowSizeChanged(GLFWwindow *win, int w, int h)
{
    std::cout << "New win size: " << w << ", " << h << std::endl;

}
