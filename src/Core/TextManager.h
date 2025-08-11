#ifndef TEXT_MANAGER_H_
#define TEXT_MANAGER_H_

#include "Renderer.h"
#include <vector>
#include <functional>
#include <array>
#include <SDL3_ttf/SDL_ttf.h>
#include "Vector2.h"
#include "TimelineProperty.hpp"
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
        Texture m_tex;
        int m_minx = 0, m_maxx = 0, m_miny = 0, m_maxy = 0, m_advance = 0;
        Symbol() = default;
        Symbol& operator=(Symbol &rhs_) = delete;
        Symbol(Symbol &rhs_) = delete;
        Symbol& operator=(Symbol &&rhs_);
        Symbol(Symbol &&rhs_);
        virtual ~Symbol();
    };

    class Font
    {
    public:
        template<typename Func, typename... Args>
        Font(Func generateSymbols_, int height_, const CharChunkDistribution &distrib_, Args&&... args_);
        const Symbol &operator[](uint32_t ch_) const;

        const int m_height;

    private:
        const CharChunkDistribution &m_distrib;
        std::vector<std::array<Symbol, CHUNK_SIZE>> m_symbols;
    };
}

class TextManager
{
public:
    TextManager(Renderer &renderer_);

    //void renderText(const std::string &text_, int fontid_, Vector2<int> pos_, fonts::HOR_ALIGN horAlign_ = fonts::HOR_ALIGN::LEFT, Camera *cam_ = nullptr);
    void renderText(const std::string &text_, int fontid_, Vector2<int> pos_, fonts::HOR_ALIGN horAlign_ = fonts::HOR_ALIGN::LEFT, Camera *cam_ = nullptr);

    const fonts::Symbol *getSymbol(int fontid_, uint32_t ch_) const;
    int getFontHeight(int fontid_) const;

private:
    //static void generateOutlinedTexturedSymbols(std::vector<std::array<fonts::Symbol, fonts::CHUNK_SIZE>> &symbolChunks_, const fonts::CharChunkDistribution &distrib_, Renderer &renderer_, const std::string &basePath_, const std::string &font_, const std::string &texture_, int size_, int outlineWidth_, const SDL_Color &outlineColor_);
    //static void generateOutlinedSymbols(std::vector<std::array<fonts::Symbol, fonts::CHUNK_SIZE>> &symbolChunks_, const fonts::CharChunkDistribution &distrib_, Renderer &renderer_, const std::string &basePath_, const std::string &font_, int size_, int outlineWidth_, const SDL_Color &color_, const SDL_Color &outlineColor_);
    static void generateSimpleSymbols(std::vector<std::array<fonts::Symbol, fonts::CHUNK_SIZE>> &symbolChunks_, const fonts::CharChunkDistribution &distrib_, Renderer &renderer_, const std::string &basePath_, const std::string &font_, int size_, const SDL_Color &color_);
    static void generateSimpleShadedSymbols(std::vector<std::array<fonts::Symbol, fonts::CHUNK_SIZE>> &symbolChunks_, const fonts::CharChunkDistribution &distrib_, Renderer &renderer_, const std::string &basePath_, const std::string &font_, int size_, const SDL_Color &color_, const SDL_Color &shadeColor_);

    fonts::CharChunkDistribution m_charChunks;
    Renderer &m_renderer;
    std::array<fonts::Font, 4> m_fonts;

};

#endif
