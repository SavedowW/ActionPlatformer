#include "GameData.h"
#include "TextManager.h"
#include "FilesystemUtils.h"
#include "Logger.hpp"  // IWYU pragma: keep
#include "SDLWrappers.h"

namespace fonts
{
    Font::Font(size_t chunkSize_, std::unique_ptr<SymbolGenerator> &&generator_) :
        m_chunkSize{chunkSize_},
        m_generator{std::move(generator_)}
    {
        if (!m_generator)
            throw std::runtime_error("Created generator is nullptr");
    }

    SymbolGenerator::SymbolGenerator(uint8_t height_) :
        m_height{height_}
    {}

    uint8_t SymbolGenerator::height() const noexcept
    {
        return m_height;
    }

    uint8_t Font::height() const noexcept
    {
        return m_generator->height();
    }

    const Symbol &Font::operator[](uint32_t char_)
    {
        const uint32_t chunk = char_ / m_chunkSize;
        const uint32_t loc = char_ - chunk * m_chunkSize;
        const auto found = m_chunks.find(chunk);

        if (found != m_chunks.end())
            return found->second[loc];

        auto res = m_chunks.emplace(chunk, Chunk(m_chunkSize));
        assert(res.second);
        m_generator->fillChunk(res.first->second, chunk * m_chunkSize);
        return res.first->second.at(loc);
    }
}


namespace TextAligners 
{
    CommonAligner::CommonAligner(const U8Wrapper &wrp_, fonts::Font &font_) noexcept :
        m_wrp{wrp_},
        m_font{font_}
    {}

    int CommonAligner::collectLength() const noexcept
    {
        const uint32_t ch1 = m_wrp.begin().getu8();

        auto len = m_font[ch1].m_minx;
        for (auto &ch : m_wrp)
        {
            const auto &sym = m_font[ch.getu8()];
            len += sym.m_advance;
        }

        return len;
    }

    AlignerLeft::AlignerLeft(const U8Wrapper &wrp_, fonts::Font &font_) noexcept :
        CommonAligner{wrp_, font_}
    {}

    Vector2<int> AlignerLeft::adjustPos(Vector2<int> pos_) const noexcept
    {
        return pos_;
    }

    AlignerCenter::AlignerCenter(const U8Wrapper &wrp_, fonts::Font &font_) noexcept :
        CommonAligner{wrp_, font_}
    {}

    Vector2<int> AlignerCenter::adjustPos(Vector2<int> pos_) const noexcept
    {
        const auto len = collectLength();

        pos_.x -= len / 2;

        return pos_;
    }

    AlignerRight::AlignerRight(const U8Wrapper &wrp_, fonts::Font &font_) noexcept :
        CommonAligner{wrp_, font_}
    {}

    Vector2<int> AlignerRight::adjustPos(Vector2<int> pos_) const noexcept
    {
        const auto len = collectLength();

        pos_.x -= len;

        return pos_;
    }
}


SymbolGeneratorSimple::SymbolGeneratorSimple(const std::string &file_, uint8_t size_, const Color &color_) :
    fonts::SymbolGenerator(size_),
    m_font{file_.c_str(), static_cast<float>(size_)},
    m_color{color_}
{}

void SymbolGeneratorSimple::fillChunk(fonts::Chunk &chunk_, uint32_t firstChar_)
{
    for (uint32_t i = 0; i < chunk_.size(); ++i)
    {
        auto &toFill = chunk_.at(i);

        const uint32_t chid_8 = firstChar_ + i;
        const uint32_t chid = utf8::u8tou32(chid_8, utf8::readCharSize(chid_8));

        if (!TTF_FontHasGlyph(m_font, chid))
            continue;

        SurfaceWrapper surf{TTF_RenderGlyph_Solid(m_font, chid, 
            {.r=m_color.ir(), .g=m_color.ig(), .b=m_color.ib(), .a=m_color.ia()})};
        SurfaceWrapper nsurf{SDL_ConvertSurface(surf, SDL_PIXELFORMAT_ABGR8888)};

        Texture tex(Texture::Config{nsurf});
        toFill.m_id = tex.handler();
        toFill.m_size = tex.size();
        tex.release();

        TTF_GetGlyphMetrics(m_font, chid, &toFill.m_minx, &toFill.m_maxx, &toFill.m_miny, &toFill.m_maxy, &toFill.m_advance);
    }
}


SymbolGeneratorShaded::SymbolGeneratorShaded(Renderer &renderer_, const std::string &file_, uint8_t size_, const Color &shadeColor_, const Color &primaryColor_) :
    fonts::SymbolGenerator(size_),
    m_font{file_.c_str(), static_cast<float>(size_)},
    m_shadeColor{shadeColor_},
    m_primaryColor{primaryColor_},
    m_renderer{renderer_}
{}

void SymbolGeneratorShaded::fillChunk(fonts::Chunk &chunk_, uint32_t firstChar_)
{
    Texture shadedTex, unshadedTex;

    for (uint32_t i = 0; i < chunk_.size(); ++i)
    {
        auto &toFill = chunk_.at(i);

        const uint32_t chid_8 = firstChar_ + i;
        const uint32_t chid = utf8::u8tou32(chid_8, utf8::readCharSize(chid_8));

        if (!TTF_FontHasGlyph(m_font, chid))
            continue;

        {
            // Outline (background)
            // TODO: do it via shader instead
            TTF_SetFontOutline(m_font, 1);
            SurfaceWrapper surf{TTF_RenderGlyph_Solid(m_font, chid, 
                {m_shadeColor.ir(), m_shadeColor.ig(), m_shadeColor.ib(), m_shadeColor.ia()})};
            SurfaceWrapper surfConverted{SDL_ConvertSurface(surf, SDL_PIXELFORMAT_ABGR8888)};

            shadedTex.init(Texture::Config{surfConverted});
        }

        {
            // Inner letter
            TTF_SetFontOutline(m_font, 0);
            SurfaceWrapper surf{TTF_RenderGlyph_Solid(m_font, chid, 
                {m_primaryColor.ir(), m_primaryColor.ig(), m_primaryColor.ib(), m_primaryColor.ia()})};
            SurfaceWrapper surfConverted{SDL_ConvertSurface(surf, SDL_PIXELFORMAT_ABGR8888)};

            unshadedTex.init(Texture::Config{surfConverted});
        }

        Texture tex(Texture::Config{shadedTex.size().add(1, 1)});
        toFill.m_id = tex.handler();
        toFill.m_size = tex.size();

        TTF_GetGlyphMetrics(m_font, chid, &toFill.m_minx, &toFill.m_maxx, &toFill.m_miny, &toFill.m_maxy, &toFill.m_advance);

        m_renderer.setTarget(tex);
        tex.release();

        m_renderer.fillRenderer(Color{255, 255, 255, 0});
        m_renderer.renderTexture(shadedTex.handler(), {0, 0}, shadedTex.size(), SDL_FLIP_VERTICAL, 1.0f);
        m_renderer.renderTexture(unshadedTex.handler(), {0, 2}, unshadedTex.size(), SDL_FLIP_VERTICAL, 1.0f);

    }
}


TextManager::TextManager(Renderer &renderer_) :
    m_renderer(renderer_),
    m_fonts{fonts::Font{256, std::make_unique<SymbolGeneratorShaded>(renderer_, Filesystem::getRootDirectory() + "/Resources/Fonts/Silkscreen.ttf", 32, Color{100, 100, 100, 255}, Color{255, 255, 255, 255})}, // Screen debug data
    fonts::Font{256, std::make_unique<SymbolGeneratorSimple>(Filesystem::getRootDirectory() + "/Resources/Fonts/Silkscreen.ttf", 10, gamedata::colors::LVL1)}, // For npc debug
    fonts::Font{256, std::make_unique<SymbolGeneratorSimple>(Filesystem::getRootDirectory() + "/Resources/Fonts/Silkscreen.ttf", 8, Color{255, 255, 255, 255})}, // For navigation system
    fonts::Font{64, std::make_unique<SymbolGeneratorSimple>(Filesystem::getRootDirectory() + "/Resources/Fonts/Silkscreen.ttf", 16, gamedata::colors::LVL1)}} // Used for chatbox
{
}

fonts::Font &TextManager::getFont(Fonts font_)
{
    return m_fonts.at(static_cast<size_t>(font_));
}
