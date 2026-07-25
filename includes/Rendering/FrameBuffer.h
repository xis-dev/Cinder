#pragma once

#include "glad/glad.h"
#include <vector>
class RenderPass
{
public:
    RenderPass() = default;
    RenderPass(const std::vector<unsigned> &colourAttachments);


    unsigned frameBuffer{};
    std::vector<unsigned> m_colourBuffers;
    unsigned depth{};
    // May not be used if stencil isnt needed or depth holds both
    unsigned stencil{};


    void attachColourBuffer(unsigned buffer, bool isTexture);

    // May be depth + stencil
    void attachDepthBuffer(unsigned buffer, bool isTexture = false, GLint attachment = GL_DEPTH_ATTACHMENT);

    void attachStencilBuffer(unsigned buffer, bool isTexture = false, GLint attachment = GL_STENCIL_INDEX8);
    
    void updateColourBuffer(int index, int width, int height, GLint format, GLint storageType, bool isTexture = true);


    void updateDepthBuffer(int width, int height, GLint attachment, GLint storageType, GLint format, bool isTexture = true);


    void updateStencilBuffer(int index, int width, int height, GLint attachment, GLint storageType, GLint format, bool isTexture = true);

    void render(int w, int h);

private:
    void attachBuffer(unsigned buffer, bool isTexture, GLint attachment);
    void updateBuffer(unsigned buffer, int width, int height, GLint attachment, GLint format, GLint storageType, bool isTexture = true);
};