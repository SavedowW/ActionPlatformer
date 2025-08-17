#include "Texture.h"
#include "glad/glad.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

void TextureResource::cleanSelf()
{
    if (m_id != 0)
    {
        std::cout << "Deleting loaded TextureResource \"" << m_name << "\"" << std::endl;
        glDeleteTextures(1, &m_id);
        m_id = 0;
    }
}

TextureResource::TextureResource(const std::string &name_, const Vector2<int> &size_, const unsigned int id_) :
    Texture(size_, id_),
    m_name(name_)
{
}

TextureResource::TextureResource(TextureResource &&tex_) noexcept :
    Texture(tex_),
    m_name(tex_.m_name)
{
    m_size = tex_.m_size;
    m_id = tex_.m_id;

    tex_.m_id = 0;
    tex_.m_size = {0, 0};
    tex_.m_name = "<DELETED>";
}

TextureResource &TextureResource::operator=(TextureResource &&tex_) noexcept
{
    cleanSelf();
        
    m_name = tex_.m_name;
    m_size = tex_.m_size;
    m_id = tex_.m_id;

    tex_.m_id = 0;
    tex_.m_size = {0, 0};
    tex_.m_name = "<DELETED>";

    return *this;
}

TextureResource::~TextureResource()
{
    cleanSelf();
}
