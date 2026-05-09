#pragma once
#include "glad/glad.h"

class Framebuffer
{
public:
    void init()
    {
        glGenFramebuffers(1, &m_framebuffer);
    }

    operator unsigned int() const noexcept
    {
        return m_framebuffer;
    }

    ~Framebuffer()
    {
        if (m_framebuffer)
            glDeleteFramebuffers(1, &m_framebuffer);
    }

private:
    unsigned int m_framebuffer = 0;
};
