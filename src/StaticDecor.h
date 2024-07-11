#ifndef STATIC_DECOR_H_
#define STATIC_DECOR_H_

#include "Application.h"

class StaticDecor
{
public:
    StaticDecor(Application *application_, int texId_, SDL_RendererFlip flip_, Vector2<float> pos_);

    void draw(const Camera &cam_);

private:
    Texture_t m_tex;
    Renderer &m_renderer;
    Vector2<float> m_pos;
    SDL_RendererFlip m_flip;

};

#endif