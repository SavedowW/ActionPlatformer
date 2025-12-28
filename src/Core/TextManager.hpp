#pragma once
#include "TextManager.h"

template<typename AlignerT>
void TextManager::renderText(const std::string &text_, int fontid_, Vector2<int> pos_, const Camera &cam_)
{
    U8Wrapper wrp(text_);
    const auto &font = m_fonts.at(fontid_);
    AlignerT aligner{wrp, font};

    pos_ = aligner.adjustPos(pos_);

    pos_.x += font[wrp.begin().getu8()].m_minx;
    for (auto &ch : wrp)
    {
        const auto &sym = font[ch.getu8()];
        m_renderer.renderTexture(sym.m_tex.m_id, pos_, sym.m_tex.m_size, SDL_FLIP_NONE, 1.0f, cam_);
        pos_.x += sym.m_advance;
    }
}

template<typename AlignerT>
void TextManager::renderText(const std::string &text_, int fontid_, Vector2<int> pos_)
{
    U8Wrapper wrp(text_);
    const auto &font = m_fonts.at(fontid_);
    AlignerT aligner{wrp, font};

    pos_ = aligner.adjustPos(pos_);

    pos_.x += font[wrp.begin().getu8()].m_minx;
    for (auto &ch : wrp)
    {
        const auto &sym = font[ch.getu8()];
        m_renderer.renderTexture(sym.m_tex.m_id, pos_, sym.m_tex.m_size, SDL_FLIP_NONE, 1.0f);
        pos_.x += sym.m_advance;
    }
}
