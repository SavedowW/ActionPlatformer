#ifndef TEXT_MANAGER_H_
#define TEXT_MANAGER_H_

#include "Texture.h"
#include "Renderer.h"
#include <concepts>
#include <vector>
#include <array>
#include <SDL3_ttf/SDL_ttf.h>
#include "Vector2.hpp"
#include "TimelineProperty.hpp"
#include "utf8.h"
#include <set>

namespace fonts
{
    inline constexpr uint32_t CHUNK_SIZE = 256;

    struct CharChunkDistribution
    {
        CharChunkDistribution(const std::string &charlist_);

        std::set<uint32_t> m_chunks;
        TimelineProperty<uint32_t, uint32_t> m_chunkSearch;
    };

    enum class HOR_ALIGN : uint8_t
    {
        LEFT,
        CENTER,
        RIGHT
    };

    struct Symbol
    {
        Texture m_tex;
        int m_minx = 0, m_maxx = 0, m_miny = 0, m_maxy = 0, m_advance = 0;
        Symbol() noexcept = default;
        Symbol& operator=(Symbol &rhs_) = delete;
        Symbol(Symbol &rhs_) = delete;
        Symbol& operator=(Symbol &&rhs_) noexcept;
        Symbol(Symbol &&rhs_) noexcept;
        virtual ~Symbol();
    };

    class Font
    {
    public:
        template<typename Func, typename... Args>
        Font(Renderer &renderer_, Func generateSymbols_, int height_, const CharChunkDistribution &distrib_, Args&&... args_)
            requires std::invocable<Func, Renderer&, std::vector<std::array<Symbol, CHUNK_SIZE>>&, decltype(distrib_), Args...>;
        const Symbol &operator[](uint32_t ch_) const;

        const int m_height;

    private:
        const CharChunkDistribution &m_distrib;
        std::vector<std::array<Symbol, CHUNK_SIZE>> m_symbols;
    };
}

namespace TextAligners 
{
    class CommonAligner
    {
    public:
        virtual Vector2<int> adjustPos(Vector2<int> pos_) const noexcept = 0;
        virtual ~CommonAligner() = default;

    protected:
        CommonAligner(const U8Wrapper &wrp_, const fonts::Font &font_) noexcept;
        int collectLength() const noexcept;

    private:
        const U8Wrapper &m_wrp;
        const fonts::Font &m_font;
    };

    class AlignerLeft : public CommonAligner
    {
    public:
        AlignerLeft(const U8Wrapper &wrp_, const fonts::Font &font_) noexcept;
        Vector2<int> adjustPos(Vector2<int> pos_) const noexcept override;
    };

    class AlignerCenter : public CommonAligner
    {
    public:
        AlignerCenter(const U8Wrapper &wrp_, const fonts::Font &font_) noexcept;
        Vector2<int> adjustPos(Vector2<int> pos_) const noexcept override;
    };

    class AlignerRight : public CommonAligner
    {
    public:
        AlignerRight(const U8Wrapper &wrp_, const fonts::Font &font_) noexcept;
        Vector2<int> adjustPos(Vector2<int> pos_) const noexcept override;
    };
} // TextAligners


class TextManager
{
private:
    using Fonts = std::array<fonts::Font, 4>;

public:
    TextManager(Renderer &renderer_);

    //void renderText(const std::string &text_, int fontid_, Vector2<int> pos_, fonts::HOR_ALIGN horAlign_ = fonts::HOR_ALIGN::LEFT, Camera *cam_ = nullptr);

    // Ignores '\n', '\t'
    template<typename AlignerT>
    void renderText(const std::string &text_, int fontid_, Vector2<int> pos_, const Camera &cam_);

    // Ignores '\n', '\t'
    template<typename AlignerT>
    void renderText(const std::string &text_, int fontid_, Vector2<int> pos_);

    const fonts::Symbol *getSymbol(int fontid_, uint32_t ch_) const;
    int getFontHeight(int fontid_) const;

private:
    //static void generateOutlinedTexturedSymbols(Renderer &renderer_, std::vector<std::array<fonts::Symbol, fonts::CHUNK_SIZE>> &symbolChunks_, const fonts::CharChunkDistribution &distrib_, const std::string &basePath_, const std::string &font_, const std::string &texture_, int size_, int outlineWidth_, const SDL_Color &outlineColor_);
    //static void generateOutlinedSymbols(Renderer &renderer_, std::vector<std::array<fonts::Symbol, fonts::CHUNK_SIZE>> &symbolChunks_, const fonts::CharChunkDistribution &distrib_, const std::string &basePath_, const std::string &font_, int size_, int outlineWidth_, const SDL_Color &color_, const SDL_Color &outlineColor_);
    static void generateSimpleSymbols(Renderer &renderer_, std::vector<std::array<fonts::Symbol, fonts::CHUNK_SIZE>> &symbolChunks_, const fonts::CharChunkDistribution &distrib_, const std::string &basePath_, const std::string &font_, int size_, const SDL_Color &color_);
    static void generateSimpleShadedSymbols(Renderer &renderer_, std::vector<std::array<fonts::Symbol, fonts::CHUNK_SIZE>> &symbolChunks_, const fonts::CharChunkDistribution &distrib_, const std::string &basePath_, const std::string &font_, int size_, const SDL_Color &color_, const SDL_Color &shadeColor_);

    fonts::CharChunkDistribution m_charChunks;
    Renderer &m_renderer;
    Fonts m_fonts;

};

#endif
