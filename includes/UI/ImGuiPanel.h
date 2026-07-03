#pragma once

#include "Rendering/ImGuiHolder.h"


#include <string>
#include "imgui.h"

class Renderer;
class Entity;
class Scene;

class ImGuiPanel
{
public:
    virtual ~ImGuiPanel() = default;

    ImGuiPanel(const std::string& name,
               ImGuiWindowFlags flags = 0,
               bool open = true):
                                  m_name(name),
                                  m_flags(flags),
                                  m_open(open){}

protected:
    std::string m_name{};
    ImGuiWindowFlags m_flags{};
    bool m_open{};
    bool shouldRender{true};

    bool beginFrame();
    virtual void update() = 0;
    void endFrame();
public:
    const std::string& getName() const;
    ImGuiWindowFlags getFlags() const;
    bool isOpen() const;
    bool canRender() const;
    void togglePanelRendering();
    void setFlags(ImGuiWindowFlags flags);
    void addFlags(ImGuiWindowFlags flags);
    virtual void render();
};

class PropertiesPanel: public ImGuiPanel
{
    Entity* m_selectedEntity{nullptr};

public:
    PropertiesPanel(const std::string &name, Delegate<Entity*>& OnItemSelected, Delegate<Entity*>& OnEntityDestroyed, ImGuiWindowFlags flags = 0, bool open = 0);


    // TODO: When add CinderObject class, change to allow property display for materials
    void setDisplayEntity(Entity* entity);
    void entityDestroyed(Entity* entity);

protected:
    void update() override;
};

class SceneHierarchyPanel: public ImGuiPanel
{
    Scene* CURRENT_SCENE;
    Entity* m_selectedEntity{nullptr};

    Delegate<Entity*>& ItemSelected_Delegate;
public:
    SceneHierarchyPanel(const std::string &name, Scene* scene, Delegate<Entity*>& OnItemSelected, ImGuiWindowFlags flags = 0, bool open = 0)
        : ImGuiPanel( name, flags | ImGuiWindowFlags_NoCollapse, open),
          CURRENT_SCENE(scene), ItemSelected_Delegate(OnItemSelected)
    {}

private:
    void renderEntityHierarchy(Entity* entity);
protected:
    virtual void update() override;

    Entity* getSelectedEntity();
};

class RenderPanel: public ImGuiPanel
{
    Renderer* RENDERER;
    int m_minWidth;
    int m_minHeight;
    float m_aspect;

    int previousWidth{-1};
    int previousHeight{-1};
public:
    RenderPanel( const std::string &name, Renderer* renderer,
                int minW, int minH, float aspect,
                ImGuiWindowFlags flags = 0, bool open = 0)
        : ImGuiPanel(name, flags | ImGuiWindowFlags_NoScrollbar, open), RENDERER(renderer),
                                         m_minWidth(minW),
                                         m_minHeight(minH), m_aspect(aspect)
    {
    }

protected:
    virtual void update() override;
};

class RenderConfigPanel: public ImGuiPanel
{
    Renderer* RENDERER;
public:
    RenderConfigPanel(const std::string &name, Renderer* renderer, ImGuiWindowFlags flags = 0, bool open = 0)
        : ImGuiPanel(name, flags, open), RENDERER(renderer)
    {}

    void update() override;
};


