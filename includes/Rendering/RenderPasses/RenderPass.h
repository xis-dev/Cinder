#pragma once

#include "FrameBuffer.h"
#include "imgui.h"
#include "vec3.hpp"
#include "ext/matrix_float4x4.hpp"

class GBuffer;
class Shader;

struct FrameContext
{
    GBuffer *gBuffer{nullptr};

    int frameWidth;
    int frameHeight;

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
    bool enabled{};

public:
    virtual void updatePassSize(int w, int h) = 0;


    virtual void imguiRender()
    {
        ImGui::Checkbox("Enabled", &enabled);
    };

    void enable() { enabled = true; }

    void disable() { enabled = false; }

    bool isEnabled() const { return enabled; }

    virtual unsigned getOutput(int index) const = 0;
};


template<typename... ConfigureParams>
class IRenderPassConfigurer : public RenderPass
{
public:
    void tryConfiguredRender(const FrameContext &ctx, ConfigureParams... params)
    {
        if (enabled)
        {
            configuredRender(ctx, params...);
        }
    }

protected:
    virtual void configuredRender(const FrameContext &ctx, ConfigureParams... params) = 0;
};
