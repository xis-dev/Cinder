//
// Created by PC on 25-Jul-26.
//

#include "../../includes/Rendering/GBuffer.h"
#include "Texture.h"

#include "glad/glad.h"

void GBuffer::setup(int width, int height)
{
    gDepth = Texture::createEmptyTex(width, height, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT);

    gColorSpec = Texture::createEmptyTex(width, height, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
    gNormal = Texture::createEmptyTex(width, height, GL_RGBA16F, GL_RGBA, GL_FLOAT);
    gMaterial = Texture::createEmptyTex(width, height, GL_RGBA16F, GL_RGBA, GL_FLOAT);

    stencilBuffer = Texture::createEmptyRenderbuffer(width, height, GL_STENCIL_INDEX8, GL_UNSIGNED_BYTE);

    frameBufferHolder = FrameBuffer(std::vector{gColorSpec, gNormal, gMaterial});
    frameBufferHolder.attachDepthBuffer(gDepth, true, GL_DEPTH_ATTACHMENT);

}

void GBuffer::update(int width, int height)
{
    frameBufferHolder.updateColourBuffer(gColorSpec, width, height, GL_RGBA, GL_UNSIGNED_BYTE, true, true);
    frameBufferHolder.updateColourBuffer(gNormal, width, height, GL_RGBA, GL_FLOAT, true, true);
    frameBufferHolder.updateColourBuffer(gMaterial, width, height, GL_RGBA, GL_FLOAT, true, true);

    frameBufferHolder.updateDepthBuffer(width, height, GL_DEPTH_ATTACHMENT, GL_FLOAT, GL_DEPTH_COMPONENT, true);
    // TODO: Move format to after texture flag so stencil doesnt need to specify
   // frameBufferHolder.updateStencilBuffer(width, height, GL_STENCIL_ATTACHMENT, GL_UNSIGNED_BYTE, GL_STENCIL, false);

}
