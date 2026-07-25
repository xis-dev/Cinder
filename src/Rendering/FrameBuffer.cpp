#include "../../includes/Rendering/RenderPass.h"

#include <iostream>

#include "Texture.h"


RenderPass::RenderPass(const std::vector<unsigned>& colourAttachments)
{
    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

    m_colourBuffers = colourAttachments;

    std::vector<GLenum> drawBuffers;

    for (int i = 0; i < colourAttachments.size(); ++i)
    {
        auto currentAttachment = GL_COLOR_ATTACHMENT0 + i;
        drawBuffers.push_back(currentAttachment);
        glBindTexture(GL_TEXTURE_2D, colourAttachments[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, currentAttachment, GL_TEXTURE_2D, colourAttachments[i], 0);

    }

    glBindTexture(GL_TEXTURE_2D, 0);

    glDrawBuffers(drawBuffers.size(), drawBuffers.data());

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "RENDER_PASS:: Incomplete framebuffer." << glCheckFramebufferStatus(GL_FRAMEBUFFER) << "\n" ;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderPass::attachColourBuffer(unsigned buffer, bool isTexture)
{
    if (m_colourBuffers.size() > GL_MAX_COLOR_ATTACHMENTS) return ;

    GLint nextAttachment = !m_colourBuffers.empty() ? GL_COLOR_ATTACHMENT0 + (m_colourBuffers.size() - 1) : GL_COLOR_ATTACHMENT0;
    std::vector<GLenum> drawBuffers;
    drawBuffers.push_back(GL_COLOR_ATTACHMENT0);

    for (int i = 1; i < m_colourBuffers.size(); ++i)
    {
        drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + i);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

    if (isTexture)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, nextAttachment, GL_TEXTURE_2D, buffer, 0);
    }
    else
    {
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, nextAttachment, GL_RENDERBUFFER, buffer);
    }

    glDrawBuffers(m_colourBuffers.size(), drawBuffers.data());

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "RENDER_PASS:: Incomplete framebuffer while adding colour buffer." << glCheckFramebufferStatus(GL_FRAMEBUFFER) << "\n" ;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);


}

void RenderPass::attachDepthBuffer(unsigned buffer, bool isTexture, GLint attachment)
{
    attachBuffer(buffer, isTexture, attachment);

    depth = buffer;
}

void RenderPass::attachStencilBuffer(unsigned buffer, bool isTexture, GLint attachment)
{
    attachBuffer(buffer, isTexture, attachment);

    stencil = buffer;
}




void RenderPass::updateColourBuffer(int index, int width, int height, GLint format, GLint storageType, bool isTexture)
{
    if (index < 0 || index >= m_colourBuffers.size())   return;


    const unsigned id = m_colourBuffers[index];
    const GLint attachment = GL_COLOR_ATTACHMENT0 + index;

    updateBuffer(id, width, height, attachment, format, storageType, isTexture);

}

void RenderPass::updateDepthBuffer(int width, int height, GLint attachment, GLint storageType, GLint format,
    bool isTexture)
{
    updateBuffer(depth, width, height, attachment, format, storageType, isTexture);
}

void RenderPass::updateStencilBuffer(int index, int width, int height, GLint attachment, GLint storageType,
    GLint format, bool isTexture)
{
    updateBuffer(stencil, width, height, attachment, format, storageType, isTexture);
}

void RenderPass::attachBuffer(unsigned buffer, bool isTexture, GLint attachment)
{
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    if (isTexture)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, buffer, 0);
    }
    else
    {
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, buffer);
    }

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "RENDER_PASS:: Incomplete framebuffer while attaching buffer." << glCheckFramebufferStatus(GL_FRAMEBUFFER) << "\n" ;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderPass::updateBuffer(unsigned buffer, int width, int height, GLint attachment, GLint format, GLint storageType, bool isTexture)
{
    if (isTexture)
    {
        glBindTexture(GL_TEXTURE_2D,buffer);
        GLint internalFormat, minFilter, magFilter, wrapMode;

        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);

        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_MIN_FILTER, &minFilter);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_MAG_FILTER, &magFilter);

        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WRAP_S, &wrapMode);

        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, storageType, nullptr);

        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, buffer, 0);

        glBindTexture(GL_TEXTURE_2D, 0);
    }
    else
    {
        glBindRenderbuffer(GL_RENDERBUFFER, buffer);

        GLint internalFormat;
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_INTERNAL_FORMAT, &internalFormat);
        glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width, height);

        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, buffer);

        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }


    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "RENDER_PASS:: Incomplete framebuffer while updating buffer" << glCheckFramebufferStatus(GL_FRAMEBUFFER) << "\n" ;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


