#pragma once

class Framebuffer
{
public:
    void init();

    operator unsigned int() const noexcept;

    ~Framebuffer();

private:
    unsigned int m_framebuffer = 0;
};
