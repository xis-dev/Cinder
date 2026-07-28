//
// Created by PC on 30-Jun-26.
//

#include "../../includes/UI/CinderEditor.h"

#include "Engine.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"


void CinderEditor::initialize(GLFWwindow *window, Scene *scene, Renderer *renderer)
{

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();

    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;


   // Engine::OnWindowResized.bindFunction(this, &ImGuiHolder::OnWindowSizeChanged);

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


void CinderEditor::startRender()
{
    // setup imgui for a new frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::DockSpaceOverViewport();

    renderMenuBar();

    std::vector<ImGuiPanel*> sortedPanelNames;
    int currentIdx = 0;
    for (auto& panel : m_panels)
    {
        panel->render();
    }


}

void CinderEditor::endRender()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


void CinderEditor::OnWindowSizeChanged(GLFWwindow *win, int w, int h)
{
    std::cout << "New win size: " << w << ", " << h << std::endl;

}
