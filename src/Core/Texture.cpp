#include "Texture.h"

Texture::Texture(SDL_Texture *tex_)
{
    if (tex_ == NULL)
        throw "Texture does not exist!\n";

    m_tex = tex_;

    SDL_QueryTexture(m_tex, NULL, NULL, &m_w, &m_h);
}

Texture::Texture()
{
    m_tex = nullptr;

    m_w = -1;
    m_h = -1;
}

Texture::Texture(Texture &&tex_)
{
    m_tex = tex_.m_tex;
    m_w = tex_.m_w;
    m_h = tex_.m_h;

    tex_.m_w = -1;
    tex_.m_h = -1;
    tex_.m_tex = nullptr;
}

Texture &Texture::operator=(Texture &&rhs_)
{
    m_tex = rhs_.m_tex;
    m_w = rhs_.m_w;
    m_h = rhs_.m_h;

    rhs_.m_w = -1;
    rhs_.m_h = -1;
    rhs_.m_tex = nullptr;

    return *this;
}

void Texture::setTexture(SDL_Texture *tex_)
{
    if (tex_ == nullptr)
        throw "Texture does not exist!\n";

    if (m_tex != nullptr)
        SDL_DestroyTexture(m_tex);

    m_tex = tex_;

    SDL_QueryTexture(m_tex, NULL, NULL, &m_w, &m_h);
}

SDL_Texture *Texture::getSprite()
{
    return m_tex;
}

Texture::~Texture()
{
    if (m_tex)
        SDL_DestroyTexture(m_tex);
}