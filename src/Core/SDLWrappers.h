#pragma once
#include <SDL3/SDL.h>

class SurfaceWrapper
{
public:
    SurfaceWrapper(SDL_Surface *surface_);
    SDL_Surface &get() const;
    ~SurfaceWrapper();

private:
    SDL_Surface *const m_surface = nullptr;
};
