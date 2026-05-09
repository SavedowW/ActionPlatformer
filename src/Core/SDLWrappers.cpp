#include "SDLWrappers.h"
#include "SDL3/SDL_oldnames.h"
#include <stdexcept>

SurfaceWrapper::SurfaceWrapper(SDL_Surface *surface_) :
    m_surface{ surface_ }
{
    if (!m_surface)
        throw std::runtime_error("Trying to wrap nullptr as a surface");
}

SDL_Surface &SurfaceWrapper::get() const
{
    return *m_surface;
}

SurfaceWrapper::~SurfaceWrapper()
{
    SDL_DestroySurface(m_surface);
}
