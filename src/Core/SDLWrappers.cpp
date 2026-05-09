#include "SDLWrappers.h"
#include <format>
#include <stdexcept>

SurfaceWrapper::SurfaceWrapper(SDL_Surface *surface_) :
    m_surface{ surface_ }
{
    if (!m_surface)
        throw std::runtime_error("Trying to wrap nullptr as a surface");
}

SDL_Surface &SurfaceWrapper::get() const noexcept
{
    return *m_surface;
}

SurfaceWrapper::operator SDL_Surface&() const noexcept
{
    return *m_surface;
}

SurfaceWrapper::operator SDL_Surface*() const noexcept
{
    return m_surface;
}

SurfaceWrapper::~SurfaceWrapper()
{
    SDL_DestroySurface(m_surface);
}


SurfaceOptional::SurfaceOptional(SDL_Surface *surface_) noexcept :
    m_surface{surface_}
{}

SurfaceOptional &SurfaceOptional::operator=(SDL_Surface *surface_) noexcept
{
    SDL_DestroySurface(m_surface);
    
    m_surface = surface_;

    return *this;
}

SurfaceOptional::operator bool() const noexcept
{
    return m_surface;
}

SurfaceWrapper SurfaceOptional::wrap()
{
    if (!m_surface)
        throw std::runtime_error("Trying to create surface wrapper from nullptr");

    auto *tmp = m_surface;
    m_surface = nullptr;

    return SurfaceWrapper{tmp};
}

SDL_Surface *SurfaceOptional::get() const noexcept
{
    return m_surface;
}

SurfaceOptional::~SurfaceOptional()
{
    SDL_DestroySurface(m_surface);
}


FontWrapper::FontWrapper(const char *file_, float size_) :
    m_font{TTF_OpenFont(file_, size_)}
{
    if (!m_font)
        throw std::runtime_error(std::format("Failed to create font: {}", SDL_GetError()));
}

FontWrapper::operator TTF_Font&() const noexcept
{
    return *m_font;
}

FontWrapper::operator TTF_Font*() const noexcept
{
    return m_font;
}

FontWrapper::~FontWrapper()
{
    TTF_CloseFont(m_font);
}