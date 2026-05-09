#pragma once
#include "Vector2.hpp"
#include "glad/glad.h"
#include <SDL3/SDL_surface.h>

struct Texture
{
public:
    class Config
    {
    public:
        Config(const Vector2<int> &size_);
        Config(const SDL_Surface &sur);

        // Default is RGBA
        Config &useRGB() noexcept;

    private:
        const Vector2<int> m_size;
        const void *m_pixels = nullptr;
        GLint m_format = GL_RGBA;

        friend Texture;
    };

    Texture() = default;
    Texture(const Texture&) = delete;
    Texture &operator=(const Texture&) = delete;
    Texture(Texture&&) noexcept;
    Texture &operator=(Texture&&) noexcept;

    Texture(const Config &cfg_);

    void init(const Config &cfg_);
    void free();

    const Vector2<int> &size() const noexcept;
    unsigned int handler() const noexcept;

    ~Texture();

private:
    Vector2<int> m_size;
    unsigned int m_id = 0;
};
