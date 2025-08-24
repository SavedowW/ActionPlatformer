#ifndef TEXTURE_H_
#define TEXTURE_H_

#include "Vector2.hpp"

class TextureManager;

namespace fonts
{
    struct Symbol;
}

struct Texture
{
    Vector2<int> m_size;
    unsigned int m_id = 0;
};

class TextureResource : public Texture
{
public:
    TextureResource() = default;
    TextureResource(const std::string &name_, const Vector2<int> &size_, unsigned int id_);
    TextureResource(const TextureResource &tex_) = delete;
    TextureResource& operator=(const TextureResource &tex_) = delete;
    TextureResource(TextureResource &&tex_) noexcept;
    TextureResource& operator=(TextureResource &&tex_) noexcept;
    ~TextureResource();

    void cleanSelf();

private:
    std::string m_name;
};

#endif
