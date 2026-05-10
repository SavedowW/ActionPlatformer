#pragma once
#include "Framebuffer.h"
#include "glad/glad.h"

void Framebuffer::init()
{
    glGenFramebuffers(1, &m_framebuffer);
}

Framebuffer::operator unsigned int() const noexcept
{
    return m_framebuffer;
}

Framebuffer::~Framebuffer()
{
    if (m_framebuffer)
        glDeleteFramebuffers(1, &m_framebuffer);
}
