#ifndef TEXT_MANAGER_H_
#define TEXT_MANAGER_H_

#include "Renderer.h"
#include <vector>
#include <functional>
#include <array>
#include <SDL_ttf.h>
#include "Vector2.h"
#include "TimelineProperty.h"
#include <set>

namespace fonts
{
    inline constexpr uint32_t CHUNK_SIZE = 256;

    struct CharChunkDistribution
    {
        CharChunkDistribution(const std::string &charlist_);

        std::set<uint32_t> m_chunks;
        TimelineProperty<uint32_t> m_chunkSearch;
    };

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
        Font(Func generateSymbols_, const CharChunkDistribution &distrib_, Args&&... args_);
        const Symbol &operator[](uint32_t ch_);

    private:
        const CharChunkDistribution &m_distrib;
        std::vector<std::array<Symbol, CHUNK_SIZE>> m_symbols;
    };
}

class TextManager
{
public:
    TextManager(Renderer *renderer_, const std::string &basePath_);

    void renderText(const std::string &text_, int fontid_, Vector2<float> pos_, fonts::HOR_ALIGN horAlign_ = fonts::HOR_ALIGN::LEFT);

private:
    static void generateOutlinedTexturedSymbols(std::vector<std::array<fonts::Symbol, fonts::CHUNK_SIZE>> &symbolChunks_, const fonts::CharChunkDistribution &distrib_, Renderer &renderer_, const std::string &basePath_, const std::string &font_, const std::string &texture_, int size_, int outlineWidth_, const SDL_Color &outlineColor_);
    static void generateOutlinedSymbols(std::vector<std::array<fonts::Symbol, fonts::CHUNK_SIZE>> &symbolChunks_, const fonts::CharChunkDistribution &distrib_, Renderer &renderer_, const std::string &basePath_, const std::string &font_, int size_, int outlineWidth_, const SDL_Color &color_, const SDL_Color &outlineColor_);
    static void generateSimpleSymbols(std::vector<std::array<fonts::Symbol, fonts::CHUNK_SIZE>> &symbolChunks_, const fonts::CharChunkDistribution &distrib_, Renderer &renderer_, const std::string &basePath_, const std::string &font_, int size_, const SDL_Color &color_);
    static void generateSimpleShadedSymbols(std::vector<std::array<fonts::Symbol, fonts::CHUNK_SIZE>> &symbolChunks_, const fonts::CharChunkDistribution &distrib_, Renderer &renderer_, const std::string &basePath_, const std::string &font_, int size_, const SDL_Color &color_, const SDL_Color &shadeColor_);

    fonts::CharChunkDistribution m_charChunks;
    Renderer *m_renderer;
    std::array<fonts::Font, 1> m_fonts;

};

#endif