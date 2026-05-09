#include "Texture.h"
#include "glad/glad.h"

Texture::Config::Config(const Vector2<int> &size_) :
    m_size{ size_ }
{}

Texture::Config::Config(const SDL_Surface &sur) :
    m_size{ sur.w, sur.h },
    m_pixels{sur.pixels}
{}

Texture::Config &Texture::Config::useRGB() noexcept
{
    m_format = GL_RGB;
    return *this;
}

Texture::Texture(const Config &cfg_)
{
    init(cfg_);
}

void Texture::init(const Config &cfg_)
{
    free();

    m_size = cfg_.m_size;

    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_2D, m_id);
    glTexImage2D(GL_TEXTURE_2D, 0, cfg_.m_format, cfg_.m_size.x, cfg_.m_size.y, 0, cfg_.m_format, GL_UNSIGNED_BYTE, cfg_.m_pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::free()
{
    if (m_id != 0)
    {
        glDeleteTextures(1, &m_id);
        m_id = 0;
    }
}

const Vector2<int> &Texture::size() const noexcept
{
    return m_size;
}

unsigned int Texture::handler() const noexcept
{
    return m_id;
}

Texture::Texture(Texture &&rhs_) noexcept :
    m_size{rhs_.m_size},
    m_id{rhs_.m_id}
{
    rhs_.m_size.x = 0;
    rhs_.m_size.y = 0;
    rhs_.m_id = 0;
}

Texture &Texture::operator=(Texture &&rhs_) noexcept
{
    free();

    m_size = rhs_.m_size;
    m_id = rhs_.m_id;

    rhs_.m_size.x = 0;
    rhs_.m_size.y = 0;
    rhs_.m_id = 0;

    return *this;
}

Texture::~Texture()
{
    free();
}
