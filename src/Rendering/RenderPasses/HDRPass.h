#pragma once

#include "BloomPass.h"
#include "RenderPass.h"

class HDRPass: public IRenderPassConfigurer<unsigned, float, const BloomPass&>
{
    Shader* m_shader;

public:
    FrameBuffer m_buffer;

    float exposure{1.0f};

    HDRPass() = default;
    HDRPass(int w, int h, Shader* shader);
public:

    void updatePassSize(int w, int h)    override;

    unsigned getOutput(int index) const override;

    void imguiRender() override;

protected:
    void configuredRender(const FrameContext &ctx, unsigned textureToApplyHDR, float gamma, const BloomPass& bloomPass) override;

};


