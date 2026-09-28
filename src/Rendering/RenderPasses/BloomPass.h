#pragma once

#include "RenderPass.h"

class DeferredLightPass;

class BloomPass: public IRenderPassConfigurer<unsigned>
{

    Shader* m_bloomShader;
    Shader* m_blurShader;

    glm::vec2 texelSize{};
    glm::vec2 scaledTexelSize{};
public:
    FrameBuffer m_buffer;
    FrameBuffer m_pingBuffer;
    FrameBuffer m_pongBuffer;

    float bloomTextureScale{0.5f};
    float bloomStrength{0.1f};

    BloomPass() = default;
    BloomPass(int w, int h, Shader *bloomShader, Shader *blurShader);

    void updatePassSize(int w, int h) override;

    unsigned getOutput(int index) const override;

    void imguiRender() override;
protected:
    void configuredRender(const FrameContext &ctx, unsigned bufferToBloom) override;

};


