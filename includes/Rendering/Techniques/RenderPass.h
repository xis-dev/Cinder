#pragma once

#include "FrameBuffer.h"
#include "imgui.h"
#include "vec3.hpp"
#include "ext/matrix_float4x4.hpp"

class GBuffer;
class Shader;

struct FrameContext
{

    GBuffer* gBuffer{nullptr};

    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;

    glm::mat4 invViewMatrix;
    glm::mat4 invProjectionMatrix;
};
class RenderPass
{
public:
    virtual ~RenderPass() = default;



protected:

    int m_width{};
    int m_height{};

    bool enabled{};

public:

    virtual void updatePassSize(int w, int h) {m_width = w; m_height = h;}


    virtual void imguiRender()
    {
        ImGui::Checkbox("Enabled", &enabled);
    };

    void enable() {enabled = true;}

    void disable() {enabled = false;}

    bool isEnabled() const {return enabled;}

    virtual unsigned getOutput(int index) const = 0;

};


// TODO: Maybe refactor to hold update function too, if any render pass needs special update params
template <typename... InitializationParams>
class IRenderPassInitializer: RenderPass
{
public:
    IRenderPassInitializer(int w, int h, InitializationParams... params)
    {
        m_width = w;
        m_height = h;
    };
};

template <typename... ConfigureParams>
class IRenderPassConfigurer: public RenderPass
{
public:

    void tryConfiguredRender(const FrameContext& ctx, ConfigureParams... params)
    {
        if (enabled)
        {
           configuredRender(ctx, params...);
        }
    }



protected:
    virtual void configuredRender(const FrameContext& ctx, ConfigureParams... params) = 0;
};


