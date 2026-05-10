#include "GameData.h"
#include "TextManager.h"
#include "FilesystemUtils.h"
#include "SDLWrappers.h"
#include <fstream>

fonts::Symbol& fonts::Symbol::operator=(fonts::Symbol &&rhs_) noexcept
{
    m_minx = rhs_.m_minx;
    m_maxx = rhs_.m_maxx;
    m_miny = rhs_.m_miny;
    m_maxy = rhs_.m_maxy;
    m_advance = rhs_.m_advance;
    
    m_tex = std::move(rhs_.m_tex);

    return *this;
}

fonts::Symbol::Symbol(fonts::Symbol &&rhs_) noexcept : 
    m_tex{std::move(rhs_.m_tex)},
    m_minx{rhs_.m_minx},
    m_maxx{rhs_.m_maxx},
    m_miny{rhs_.m_miny},
    m_maxy{rhs_.m_maxy},
    m_advance{rhs_.m_advance}
{}

template<typename Func, typename... Args>
fonts::Font::Font(Renderer &renderer_, Func generateSymbols_, int height_, const CharChunkDistribution &distrib_, Args&&... args_)
    requires std::invocable<Func, Renderer&, std::vector<std::array<Symbol, CHUNK_SIZE>>&, decltype(distrib_), Args...> :
    m_height(height_),
    m_distrib(distrib_)
{
    std::cout << "Initializing font..." << std::endl;
    generateSymbols_(renderer_, m_symbols, distrib_, std::forward<Args>(args_)...);
}

const fonts::Symbol &fonts::Font::operator[](uint32_t ch_) const
{
    return m_symbols[m_distrib.m_chunkSearch[ch_]].at(ch_ % CHUNK_SIZE);
}


namespace TextAligners 
{
    CommonAligner::CommonAligner(const U8Wrapper &wrp_, const fonts::Font &font_) noexcept :
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

    AlignerLeft::AlignerLeft(const U8Wrapper &wrp_, const fonts::Font &font_) noexcept :
        CommonAligner{wrp_, font_}
    {}

    Vector2<int> AlignerLeft::adjustPos(Vector2<int> pos_) const noexcept
    {
        return pos_;
    }

    AlignerCenter::AlignerCenter(const U8Wrapper &wrp_, const fonts::Font &font_) noexcept :
        CommonAligner{wrp_, font_}
    {}

    Vector2<int> AlignerCenter::adjustPos(Vector2<int> pos_) const noexcept
    {
        const auto len = collectLength();

        pos_.x -= len / 2;

        return pos_;
    }

    AlignerRight::AlignerRight(const U8Wrapper &wrp_, const fonts::Font &font_) noexcept :
        CommonAligner{wrp_, font_}
    {}

    Vector2<int> AlignerRight::adjustPos(Vector2<int> pos_) const noexcept
    {
        const auto len = collectLength();

        pos_.x -= len;

        return pos_;
    }
}


// TODO: remove font size duplication
TextManager::TextManager(Renderer &renderer_) :
    m_charChunks(Filesystem::getRootDirectory() + "Resources/GeneralCharacterList.txt"),
    m_renderer(renderer_),
    m_fonts{fonts::Font(renderer_, generateSimpleShadedSymbols, 32, m_charChunks, Filesystem::getRootDirectory(), "/Resources/Fonts/Silkscreen.ttf",  32, Color{255, 255, 255, 255}, Color{100, 100, 100, 255}), // Screen debug data
    fonts::Font(renderer_, generateSimpleSymbols, 10, m_charChunks, Filesystem::getRootDirectory(), "/Resources/Fonts/Silkscreen.ttf",  10, gamedata::colors::LVL1), // For npc debug
    fonts::Font(renderer_, generateSimpleSymbols, 8, m_charChunks, Filesystem::getRootDirectory(), "/Resources/Fonts/Silkscreen.ttf",  8, Color{255, 255, 255, 255}), // For navigation system
    fonts::Font(renderer_, generateSimpleSymbols, 16, m_charChunks, Filesystem::getRootDirectory(), "/Resources/Fonts/Silkscreen.ttf",  16, gamedata::colors::LVL1)} // Used for chatbox
{
}


void TextManager::generateSimpleSymbols(Renderer&, std::vector<std::array<fonts::Symbol, fonts::CHUNK_SIZE>> &symbolChunks_, const fonts::CharChunkDistribution &distrib_, const std::string &basePath_, const std::string &font_, int size_, const Color &color_)
{
    std::cout << "Run " << __func__ << " generator" << std::endl;
    FontWrapper font((basePath_ + font_).c_str(), size_);

    int notProvided = 0;
    int charsTotal = 0;

    for (const auto &chunkInitVal : distrib_.m_chunks)
    {
        std::array<fonts::Symbol, fonts::CHUNK_SIZE> symbols_;
        for (auto i = 0u; i < fonts::CHUNK_SIZE; ++i)
        {
            charsTotal++;
            uint32_t chid_8 = chunkInitVal * fonts::CHUNK_SIZE + i;
            uint32_t chid = utf8::u8tou32(chid_8, utf8::readCharSize(chid_8));

            if (!TTF_FontHasGlyph(font, chid))
            {
                notProvided++;
                continue;
            }

            SurfaceWrapper surf{TTF_RenderGlyph_Solid(font, chid, 
                {color_.ir(), color_.ig(), color_.ib(), color_.ia()})};
            SurfaceWrapper nsurf{SDL_ConvertSurface(surf, SDL_PIXELFORMAT_ABGR8888)};

            symbols_.at(i).m_tex.init(Texture::Config{nsurf});

            TTF_GetGlyphMetrics(font, chid, &symbols_.at(i).m_minx, &symbols_.at(i).m_maxx, &symbols_.at(i).m_miny, &symbols_.at(i).m_maxy, &symbols_.at(i).m_advance);
        }

        symbolChunks_.push_back(std::move(symbols_));
    }

    std::cout << charsTotal - notProvided << " characters generated out of " << charsTotal << ", " << notProvided << " characters not provided" << std::endl;
}

void TextManager::generateSimpleShadedSymbols(Renderer &renderer_, std::vector<std::array<fonts::Symbol, fonts::CHUNK_SIZE>> &symbolChunks_, const fonts::CharChunkDistribution &distrib_, const std::string &basePath_, const std::string &font_, int size_, const Color &color_, const Color &shadeColor_)
{
    std::cout << "Run " << __func__ << " generator" << std::endl;
    FontWrapper font((basePath_ + font_).c_str(), size_);

    int notProvided = 0;
    int charsTotal = 0;

    for (const auto &chunkInitVal : distrib_.m_chunks)
    {
        std::array<fonts::Symbol, fonts::CHUNK_SIZE> symbols_;
        for (auto i = 0u; i < fonts::CHUNK_SIZE; ++i)
        {
            charsTotal++;
            uint32_t chid_8 = chunkInitVal * fonts::CHUNK_SIZE + i;
            uint32_t chid = utf8::u8tou32(chid_8, utf8::readCharSize(chid_8)); // TODO: update in main project

            if (!TTF_FontHasGlyph(font, chid))
            {
                notProvided++;
                continue;
            }


            Texture shadedTex;
            Texture unshadedTex;

            {
                // Outline (background)
                TTF_SetFontOutline(font, 1);
                SurfaceWrapper surf{TTF_RenderGlyph_Solid(font, chid, 
                    {shadeColor_.ir(), shadeColor_.ig(), shadeColor_.ib(), shadeColor_.ia()})};
                SurfaceWrapper surfConverted{SDL_ConvertSurface(surf, SDL_PIXELFORMAT_ABGR8888)};

                shadedTex.init(Texture::Config{surfConverted});
            }

            {
                // Inner letter
                TTF_SetFontOutline(font, 0);
                SurfaceWrapper surf{TTF_RenderGlyph_Solid(font, chid, 
                    {color_.ir(), color_.ig(), color_.ib(), color_.ia()})};
                SurfaceWrapper surfConverted{SDL_ConvertSurface(surf, SDL_PIXELFORMAT_ABGR8888)};

                unshadedTex.init(Texture::Config{surfConverted});
            }

            symbols_.at(i).m_tex.init(Texture::Config{shadedTex.size().add(1, 1)});
            TTF_GetGlyphMetrics(font, chid, &symbols_.at(i).m_minx, &symbols_.at(i).m_maxx, &symbols_.at(i).m_miny, &symbols_.at(i).m_maxy, &symbols_.at(i).m_advance);

            renderer_.setTarget(symbols_.at(i).m_tex);
            renderer_.fillRenderer(Color{255, 255, 255, 0});
            renderer_.renderTexture(shadedTex.handler(), {0, 0}, shadedTex.size(), SDL_FLIP_VERTICAL, 1.0f);
            renderer_.renderTexture(unshadedTex.handler(), {0, 2}, unshadedTex.size(), SDL_FLIP_VERTICAL, 1.0f);

            renderer_.resetTarget();
        }
        symbolChunks_.push_back(std::move(symbols_));
    }

    std::cout << charsTotal - notProvided << " characters generated out of " << charsTotal << ", " << notProvided << " characters not provided" << std::endl;
}

const fonts::Symbol *TextManager::getSymbol(int fontid_, uint32_t ch_) const
{
    return &m_fonts.at(fontid_)[ch_];
}

int TextManager::getFontHeight(int fontid_) const
{
    return m_fonts.at(fontid_).m_height;
}

fonts::CharChunkDistribution::CharChunkDistribution(const std::string &charlist_)
{
    std::ifstream charlist(charlist_);
    if (!charlist.is_open())
        throw std::runtime_error("Failed to open charlist: \"" + charlist_ + "\"");

    std::string s = "";
    while (std::getline(charlist, s))
    {
        U8Wrapper wrp(s);
        for (auto &el : wrp)
        {
            m_chunks.insert(el.getu8() / CHUNK_SIZE);
        }
    }

    charlist.close();

    std::cout << m_chunks.size() << " chunks, " << m_chunks.size() * CHUNK_SIZE << " characters total" << std::endl;

    int i = 0;
    for (auto &el : m_chunks)
        m_chunkSearch.addPair(el * CHUNK_SIZE, i++);
}
