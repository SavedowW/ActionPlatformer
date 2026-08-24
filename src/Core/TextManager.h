#pragma once
#include "SDLWrappers.h"
#include "utf8.h"
#include "Renderer.h"
#include "Vector2.hpp"
#include <vector>
#include <array>
#include <memory>

namespace fonts
{
    // Doesn't own anything
    struct Symbol
    {
        unsigned int m_id = 0;
        Vector2<int> m_size;
        int m_minx = 0, m_maxx = 0, m_miny = 0, m_maxy = 0, m_advance = 0;
    };

    using Chunk = std::vector<Symbol>;

    class SymbolGenerator
    {
    public:
        SymbolGenerator(uint8_t height_);
        virtual void fillChunk(Chunk &chunk_, uint32_t firstChar_) = 0;
        uint8_t height() const noexcept;
        virtual ~SymbolGenerator() = default;

    protected:
        const uint8_t m_height;
    };

    class Font
    {
    public:
        Font(size_t chunkSize_, std::unique_ptr<SymbolGenerator> &&generator_);
        uint8_t height() const noexcept;
        const Symbol &operator[](uint32_t char_);

    private:
        const size_t m_chunkSize;
        std::unordered_map<uint32_t, std::vector<Symbol>> m_chunks;
        const std::unique_ptr<SymbolGenerator> m_generator;
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
        CommonAligner(const U8Wrapper &wrp_, fonts::Font &font_) noexcept;
        int collectLength() const noexcept;

    private:
        const U8Wrapper &m_wrp;
        fonts::Font &m_font;
    };

    class AlignerLeft : public CommonAligner
    {
    public:
        AlignerLeft(const U8Wrapper &wrp_, fonts::Font &font_) noexcept;
        Vector2<int> adjustPos(Vector2<int> pos_) const noexcept override;
    };

    class AlignerCenter : public CommonAligner
    {
    public:
        AlignerCenter(const U8Wrapper &wrp_, fonts::Font &font_) noexcept;
        Vector2<int> adjustPos(Vector2<int> pos_) const noexcept override;
    };

    class AlignerRight : public CommonAligner
    {
    public:
        AlignerRight(const U8Wrapper &wrp_, fonts::Font &font_) noexcept;
        Vector2<int> adjustPos(Vector2<int> pos_) const noexcept override;
    };
} // TextAligners

class SymbolGeneratorSimple : public fonts::SymbolGenerator
{
public:
    SymbolGeneratorSimple(const std::string &file_, uint8_t size_, const Color &color_);

    void fillChunk(fonts::Chunk &chunk_, uint32_t firstChar_) override;

private:
    FontWrapper m_font;
    const Color m_color;
};

class SymbolGeneratorShaded : public fonts::SymbolGenerator
{
public:
    SymbolGeneratorShaded(Renderer &renderer_, const std::string &file_, uint8_t size_, const Color &shadeColor_, const Color &primaryColor_);
    void fillChunk(fonts::Chunk &chunk_, uint32_t firstChar_) override;

private:
    FontWrapper m_font;
    const Color m_shadeColor;
    const Color m_primaryColor;
    Renderer &m_renderer;
};


enum class Fonts : uint8_t
{
    DBG_UI = 0,
    DBG_NPC,
    DBG_NAVSYS,
    CHATBOX,
    NONE
};


class TextManager
{
private:
    using FontsContainer = std::array<fonts::Font, static_cast<size_t>(Fonts::NONE)>;

public:
    TextManager(Renderer &renderer_);

    // Ignores '\n', '\t'
    template<typename AlignerT>
    void renderText(const std::string &text_, Fonts font_, Vector2<int> pos_, const Camera &cam_);

    // Ignores '\n', '\t'
    template<typename AlignerT>
    void renderText(const std::string &text_, Fonts font_, Vector2<int> pos_);

    fonts::Font &getFont(Fonts font_);

private:
    Renderer &m_renderer;
    FontsContainer m_fonts;
};
