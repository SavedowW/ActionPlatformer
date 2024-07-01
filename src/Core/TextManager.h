#ifndef TEXT_MANAGER_H_
#define TEXT_MANAGER_H_

#include "Renderer.h"
#include <vector>
#include <functional>
#include <array>
#include <SDL_ttf.h>
#include "Vector2.h"

namespace fonts
{
    enum class HOR_ALIGN {LEFT, CENTER, RIGHT};

    struct Symbol
    {
        SDL_Texture *m_tex = nullptr;
        int m_minx, m_maxx, m_miny, m_maxy, m_advance;
        int m_w, m_h;
        Symbol() = default;
        Symbol& operator=(Symbol &rhs_) = delete;
        Symbol(Symbol &rhs_) = delete;
        Symbol& operator=(Symbol &&rhs_);
        Symbol(Symbol &&rhs_);
        ~Symbol();
    };

    class Font
    {
    public:
        template<typename Func, typename... Args>
        Font(Func generateSymbols_, Args&&... args_);
        const Symbol &operator[](char ch_);

    private:
        std::array<Symbol, 256> m_symbols;
    };
}

class TextManager
{
public:
    TextManager(Renderer *renderer_, const std::string &basePath_);

    void renderText(const std::string &text, int fontid, Vector2<float> pos, fonts::HOR_ALIGN horAlign_ = fonts::HOR_ALIGN::LEFT);

    SDL_Texture* renderStraight(const std::string &s_);

private:
    static void generateOutlinedTexturedSymbols(std::array<fonts::Symbol, 256> &symbols_, Renderer &renderer_, const std::string &basePath_, const std::string &font_, const std::string &texture_, int size_, int outlineWidth_, const SDL_Color &outlineColor_);
    static void generateOutlinedSymbols(std::array<fonts::Symbol, 256> &symbols_, Renderer &renderer_, const std::string &basePath_, const std::string &font_, int size_, int outlineWidth_, const SDL_Color &color_, const SDL_Color &outlineColor_);
    static void generateSimpleSymbols(std::array<fonts::Symbol, 256> &symbols_, Renderer &renderer_, const std::string &basePath_, const std::string &font_, int size_, const SDL_Color &color_);
    static void generateSimpleShadedSymbols(std::array<fonts::Symbol, 256> &symbols_, Renderer &renderer_, const std::string &basePath_, const std::string &font_, int size_, const SDL_Color &color_, const SDL_Color &shadeColor_);
    Renderer *m_renderer;
    std::array<fonts::Font, 1> m_fonts;

};

#endif