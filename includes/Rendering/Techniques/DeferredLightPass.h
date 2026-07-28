#pragma once

#include "RenderPass.h"
#include "Scene.h"
#include "SSAORenderPass.h"

class DeferredLightPass: public IRenderPassConfigurer<const SSAORenderPass&, unsigned,
                                                    Scene*, const Camera&>
{
private:
    Shader* m_shader{nullptr};
public:
    DeferredLightPass() = default;
    DeferredLightPass(int w, int h, Shader *shader);

    FrameBuffer buffer;
    bool useBlinn{true};

    unsigned getOutput(int index) const override;

    void updatePassSize(int w, int h) override;

protected:
    virtual void configuredRender(const FrameContext &ctx, const SSAORenderPass& ssaoPass, unsigned shadowMapBuffer,
                                  Scene* scene, const Camera& cam) override;
};


