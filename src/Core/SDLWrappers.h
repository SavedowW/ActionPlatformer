#pragma once
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3/SDL.h>

/*
 * Guarantees sole ownership unless handler is misused.
 * Guarantees that surface exists, throws exception otherwise.
 * Automatically destroys surface.
 */
class SurfaceWrapper
{
public:
    SurfaceWrapper(SDL_Surface *surface_);
    SDL_Surface &get() const noexcept;
    operator SDL_Surface&() const noexcept;
    operator SDL_Surface*() const noexcept;
    ~SurfaceWrapper();

private:
    SDL_Surface *const m_surface = nullptr;
};

/*
 * Guarantees sole ownership unless handler is misused.
 * Automatically destroys surface, but doesn't guarantee that it exists.
 */
class SurfaceOptional
{
public:
    SurfaceOptional(SDL_Surface *surface_) noexcept;
    SurfaceOptional &operator=(SDL_Surface *surface_) noexcept;

    [[nodiscard]]
    operator bool() const noexcept;

    [[nodiscard]]
    SurfaceWrapper wrap();

    // No guarantee that it's not a nullptr
    [[nodiscard]]
    SDL_Surface *get() const noexcept;

    ~SurfaceOptional();

private:
    SDL_Surface *m_surface = nullptr;
};

/*
 * Guarantees sole ownership unless handler is misused.
 * Guarantees that font exists, throws exception otherwise.
 * Automatically destroys font.
 */
class FontWrapper
{
public:
    FontWrapper(const char *file_, float size_);
    operator TTF_Font&() const noexcept;
    operator TTF_Font*() const noexcept;
    ~FontWrapper();

private:
    TTF_Font *const m_font = nullptr;
};
