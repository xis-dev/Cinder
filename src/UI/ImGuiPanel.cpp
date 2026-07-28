

#include "includes/UI/ImGuiPanel.h"

#include "Engine.h"
#include "Renderer.h"
#include "Scene.h"

bool ImGuiPanel::isOpen() const
{
    return m_open;
}

bool ImGuiPanel::canRender() const
{
    return shouldRender;
}

bool ImGuiPanel::beginFrame()
{
  return ImGui::Begin(m_name.c_str(), &m_open, m_flags);
}

void ImGuiPanel::update()
{
    ImVec2 size = ImGui::GetContentRegionAvail();
    panelSize = {size.x, size.y};
}


void ImGuiPanel::endFrame()
{
    ImGui::End();
}

const std::string & ImGuiPanel::getName() const
{
    return m_name;
}

ImGuiWindowFlags ImGuiPanel::getFlags() const
{
    return m_flags;
}

void ImGuiPanel::togglePanelRendering()
{
    shouldRender = !shouldRender;
}

void ImGuiPanel::setFlags(ImGuiWindowFlags flags)
{
    m_flags = flags;
}

void ImGuiPanel::addFlags(ImGuiWindowFlags flags)
{
    m_flags |= flags;
}

void ImGuiPanel::render()
{
    if (!shouldRender) return;
    if (beginFrame())
    {
        update();
    }
    endFrame();
}


PropertiesPanel::PropertiesPanel(const std::string &name, Delegate<Entity *>& OnItemSelected, Delegate<Entity *>& OnEntityDestroyed,
    ImGuiWindowFlags flags, bool open): ImGuiPanel(name, flags, open)
{
    OnItemSelected.bindFunction(this, &PropertiesPanel::setDisplayEntity);
    OnEntityDestroyed.bindFunction(this, &PropertiesPanel::entityDestroyed);
}

void PropertiesPanel::setDisplayEntity(Entity *entity)
{
    m_selectedEntity = entity;
}

void PropertiesPanel::entityDestroyed(Entity *entity)
{
    if (entity == m_selectedEntity)
    {
        m_selectedEntity = nullptr;
    }
}

void PropertiesPanel::update()
{
    ImGuiPanel::update();
    if (m_selectedEntity)
    {
        m_selectedEntity->imguiDraw();
    }
}

void SceneHierarchyPanel::
renderEntityHierarchy(Entity *entity)
{
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;

    if (!entity->getParent()) flags |= ImGuiTreeNodeFlags_DefaultOpen; // Start root component open
    if (entity->getChildren().empty()) flags |= ImGuiTreeNodeFlags_Bullet; // Bullet point on end components
    if (entity == m_selectedEntity) flags |= ImGuiTreeNodeFlags_Selected;

    bool open = ImGui::TreeNodeEx(entity->getTag(), flags);

    if (ImGui::IsItemClicked())
    {
        m_selectedEntity = entity;
        ItemSelected_Delegate.broadcast(m_selectedEntity);
    }
    else if (ImGui::BeginPopupContextItem()) // Right click selects item then popup to destroy
    {
        m_selectedEntity = entity;
        ItemSelected_Delegate.broadcast(m_selectedEntity);
        if (m_selectedEntity->getParent())
        {
            if (ImGui::Button("Destroy"))
            {
                CURRENT_SCENE->destroyEntity(m_selectedEntity);
            }
        }

        ImGui::EndPopup();
    }

    if (open)
    {
        for (auto* child: entity->getChildren())
        {
            renderEntityHierarchy(child);
        }
        ImGui::TreePop();
    }
}

void SceneHierarchyPanel::update()
{
    ImGuiPanel::update();
    if (ImGui::Button("Add Entity"))
    {
        if (ImGui::BeginPopup("add_entity_type"))
        {
           if (ImGui::BeginMenu("Mesh Entity"))
           {
            ImGui::EndMenu();

           }
            ImGui::EndPopup();
        }
    }
    renderEntityHierarchy(CURRENT_SCENE->getRoot());
}

Entity * SceneHierarchyPanel::getSelectedEntity()
{
    return m_selectedEntity;
}

void RenderPanel::update()
{
    ImGuiPanel::update();
    ImVec2 viewportSize = ImGui::GetContentRegionAvail();

    float viewW = viewportSize.x;
    float viewH = viewW / m_aspect;

    if (viewH > viewportSize.y)
    {
        viewH = viewportSize.y;
        viewW = viewH * m_aspect;
    }

    viewW = std::max(viewW, static_cast<float>(m_minWidth));
    viewH = std::max(viewH, static_cast<float>(m_minHeight));

    if (viewW != previousWidth || viewH != previousHeight)
    {
        RENDERER->changeViewportSize(viewW, viewH);
    }
    ImVec2 cursorPosOffset = ImVec2(viewportSize.x - viewW, viewportSize.y - viewH) * 0.5;
    ImGui::SetCursorPos(ImGui::GetCursorPos() + cursorPosOffset);
    ImGui::Image((ImTextureID)(intptr_t)RENDERER->getFinalSceneTexture(), ImVec2(viewW, viewH), ImVec2(0, 1), ImVec2(1, 0)); // Change uv coords since y is down in opengl
}

void RenderConfigPanel::update()
{
    ImGuiPanel::update();
    // ImGui::DragFloat("Camera Speed", &camera.m_speed, 1.0f);
    // ImGui::DragFloat("Camera Far Plane", &camera.m_farPlane, 0.1f);
    // ImGui::DragFloat("Camera Near Plane", &camera.m_nearPlane, 0.01f);
    ImGui::DragFloat("Gamma Correction exp", &RENDERER->gamma, 0.1f);
    ImGui::DragFloat("Parallax Map Height", &RENDERER->parallaxScale, 0.1f);
    ImGui::DragFloat("HDR Exposure", &RENDERER->hdrExposure, 0.1f);
    ImGui::Checkbox("SSAO", &RENDERER->useSSAO);
    ImGui::Checkbox("Grid", &RENDERER->drawGrid);
    ImGui::Checkbox("HDR", &RENDERER->hdr);
    ImGui::Checkbox("Bloom", &RENDERER->bloom);
    RENDERER->m_SSAOPass.imguiRender();
    ImGui::Checkbox("Draw Cubemap", &RENDERER->cubeMapEnabled);
    ImGui::Checkbox("Enable Backface Culling", &RENDERER->cullBackface);
    ImGui::Checkbox("Draw Wireframe", &RENDERER->drawWireframe);
    std::string deltaTimeText = "Delta Time: " + std::to_string(static_cast<int>(std::floor(float(1000 / (1 / Engine::get()->getDeltaTime()))))) + " ms/s";
    std::string frameRateText = "FPS: " + std::to_string(static_cast<int>(std::floor((float(1 / Engine::get()->getDeltaTime())))));
    ImGui::Text(deltaTimeText.c_str());
    ImGui::Text(frameRateText.c_str());
}

