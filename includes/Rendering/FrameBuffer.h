#pragma once

#include "glad/glad.h"
#include <vector>
class FrameBuffer
{
public:
    FrameBuffer() = default;
    FrameBuffer(const std::vector<unsigned> &colourAttachments);

    void attachColourBuffer(unsigned buffer, bool isTexture);

    // May be depth + stencil
    void attachDepthBuffer(unsigned buffer, bool isTexture = false, GLint attachment = GL_DEPTH_ATTACHMENT);

    void attachStencilBuffer(unsigned buffer, bool isTexture = false, GLint attachment = GL_STENCIL_ATTACHMENT);

    /**
     * @param  lookForIndexAsBuffer Rather than direct indexing of buffer, the buffer itself can be passed in and be searched for
     * @return Success state
     */
    bool updateColourBuffer(int index, int width, int height, GLint format, GLint storageType, bool isTexture = true, bool lookForIndexAsBuffer = false);


    void updateDepthBuffer(int width, int height, GLint attachment, GLint storageType, GLint format, bool isTexture = true);


    void updateStencilBuffer(int width, int height, GLint attachment, GLint storageType, GLint format, bool isTexture = true);

    unsigned getColourBuffer(int index = 0) const;
    unsigned getFrameBuffer() const;

private:

    unsigned frameBuffer{};
    std::vector<unsigned> m_colourBuffers;
    unsigned depth{};
    // May not be used if stencil isnt needed or depth holds both
    unsigned stencil{};

    void attachBuffer(unsigned buffer, bool isTexture, GLint attachment);
    void updateBuffer(unsigned buffer, int width, int height, GLint attachment, GLint format, GLint storageType, bool isTexture = true);
};

