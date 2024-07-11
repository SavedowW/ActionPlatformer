#include "StaticDecor.h"

StaticDecor::StaticDecor(Application *application_, int texId_, SDL_RendererFlip flip_, Vector2<float> pos_) :
    m_renderer(*application_->getRenderer()),
    m_pos(pos_),
    m_flip(flip_)
{
    auto &texmgmt = *application_->getTextureManager();
    m_tex = texmgmt.getTexture(texId_);
}

void StaticDecor::draw(const Camera &cam_)
{
    auto *tex = m_tex.get();
    m_renderer.renderTexture(tex->getSprite(), m_pos.x - tex->m_w / 2, m_pos.y - tex->m_h / 2, tex->m_w, tex->m_h, cam_, 0, m_flip);
}
