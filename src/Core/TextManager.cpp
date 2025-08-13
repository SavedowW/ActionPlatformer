#include "GameData.h"
#include "TextManager.h"
#include "FilesystemUtils.h"
#include "utf8.h"
#include <fstream>

fonts::Symbol& fonts::Symbol::operator=(fonts::Symbol &&rhs_)
{
    m_minx = rhs_.m_minx;
    m_maxx = rhs_.m_maxx;
    m_miny = rhs_.m_miny;
    m_maxy = rhs_.m_maxy;
    m_advance = rhs_.m_advance;
    
    m_tex = rhs_.m_tex;
    rhs_.m_tex.m_id = 0;

    return *this;
}

fonts::Symbol::Symbol(fonts::Symbol &&rhs_)
{
    m_minx = rhs_.m_minx;
    m_maxx = rhs_.m_maxx;
    m_miny = rhs_.m_miny;
    m_maxy = rhs_.m_maxy;
    m_advance = rhs_.m_advance;
    
    m_tex = rhs_.m_tex;
    rhs_.m_tex.m_id = 0;
}

fonts::Symbol::~Symbol()
{
    glDeleteTextures(1, &m_tex.m_id);
}

template<typename Func, typename... Args>
fonts::Font::Font(Func generateSymbols_, int height_, const CharChunkDistribution &distrib_, Args&&... args_) :
    m_height(height_),
    m_distrib(distrib_)
{
    std::cout << "Initializing font..." << std::endl;
    static_assert(std::is_invocable_v<Func, decltype(m_symbols)&, decltype(distrib_), Args...>, "Function Func should be invokable with these arguments");
    generateSymbols_(m_symbols, distrib_, std::forward<Args>(args_)...);
}

const fonts::Symbol &fonts::Font::operator[](uint32_t ch_) const
{
    return m_symbols[m_distrib.m_chunkSearch[ch_]][ch_ % CHUNK_SIZE];
}

// TODO: remove font size duplication
TextManager::TextManager(Renderer &renderer_) :
    m_charChunks(Filesystem::getRootDirectory() + "Resources/GeneralCharacterList.txt"),
    m_renderer(renderer_),
    m_fonts{fonts::Font(generateSimpleShadedSymbols, 12, m_charChunks, renderer_, Filesystem::getRootDirectory(), "/Resources/Fonts/Silkscreen.ttf",  12, SDL_Color{255, 255, 255, 255}, SDL_Color{100, 100, 100, 255}), // Screen debug data
    fonts::Font(generateSimpleSymbols, 10, m_charChunks, renderer_, Filesystem::getRootDirectory(), "/Resources/Fonts/Silkscreen.ttf",  10, gamedata::colors::LVL1), // For npc debug
    fonts::Font(generateSimpleSymbols, 8, m_charChunks, renderer_, Filesystem::getRootDirectory(), "/Resources/Fonts/Silkscreen.ttf",  8, SDL_Color{255, 255, 255, 255}), // For navigation system
    fonts::Font(generateSimpleSymbols, 16, m_charChunks, renderer_, Filesystem::getRootDirectory(), "/Resources/Fonts/Silkscreen.ttf",  16, gamedata::colors::LVL1)} // Used for chatbox
    //m_fonts{fonts::Font(generateOutlinedSymbols, m_charChunks, *renderer_, basePath_, "/Resources/Fonts/Silkscreen.ttf",  24, 2, SDL_Color{0, 0, 0, 255}, SDL_Color{0, 100, 0, 255})}
    //m_fonts{fonts::Font(generateOutlinedTexturedSymbols, m_charChunks, *renderer_, basePath_, "/Resources/Fonts/Silkscreen.ttf", "/Resources/Fonts/fontBack.png",  24, 2, SDL_Color{0, 0, 0, 255})}
{
}


/*void TextManager::generateOutlinedTexturedSymbols(std::vector<std::array<fonts::Symbol, fonts::CHUNK_SIZE>> &symbolChunks_, const fonts::CharChunkDistribution &distrib_, Renderer &renderer_, const std::string &basePath_, const std::string &font_, const std::string &texture_, int size_, int outlineWidth_, const SDL_Color &outlineColor_)
{
    std::cout << "Run " << __func__ << " generator\n";
    TTF_Font *font = TTF_OpenFont((basePath_ + font_).c_str(), size_);
    if (font == nullptr)
    {
            std::cout << "Cannot create font: " << SDL_GetError() << std::endl;
            return;
    }

    int notProvided = 0;
    int charsTotal = 0;
    int generated = 0;

    auto *grad = renderer_.loadTexture((basePath_ + texture_).c_str());
    SDL_SetTextureBlendMode(grad, SDL_BLENDMODE_MUL);
    auto *sdlrenderer = renderer_.getRenderer();

    for (const auto &chunkInitVal : distrib_.m_chunks)
    {
        std::array<fonts::Symbol, fonts::CHUNK_SIZE> symbols_;
        for (auto i = 0; i < fonts::CHUNK_SIZE; ++i)
        {
            charsTotal++;
            symbols_[i].m_tex = nullptr;
            uint32_t chid_8 = chunkInitVal * fonts::CHUNK_SIZE + i;
            uint32_t chid = utf8::u8tou32(chid_8, utf8::readCharSize(chid_8));

            if (!TTF_FontHasGlyph(font, chid))
            {
                notProvided++;
                continue;
            }

            TTF_SetFontOutline(font, outlineWidth_);

            // Outline (background)
            auto surf = TTF_RenderGlyph_Solid(font, chid, outlineColor_);
            if (surf == nullptr)
            {
                std::cout << "Cannot create 1st surface: " << SDL_GetError() << std::endl;
                continue;
            }
            auto *text1 = SDL_CreateTextureFromSurface(sdlrenderer, surf);
            SDL_DestroySurface(surf);

            // Inner letter
            TTF_SetFontOutline(font, 0);
            surf = TTF_RenderGlyph_Solid(font, chid, {255, 255, 255, 255});
            if (surf == nullptr)
            {
                std::cout << "Cannot create 2nd surface: " << SDL_GetError() << std::endl;
                continue;
            }
            else
                generated++;
            auto *text2 = SDL_CreateTextureFromSurface(sdlrenderer, surf);
            SDL_DestroySurface(surf);

            // Outline size
            int w1, h1;
            SDL_QueryTexture(text1, NULL, NULL, &w1, &h1);

            // Inner size
            int w2, h2;
            SDL_QueryTexture(text2, NULL, NULL, &w2, &h2);
            SDL_Rect dst;
            dst.x = outlineWidth_;
            dst.y = outlineWidth_;
            dst.w = w2;
            dst.h = h2;

            // Coloring inner letter
            auto coloredTex = renderer_.createTexture(w2, h2);
            SDL_SetTextureBlendMode(coloredTex, SDL_BLENDMODE_BLEND);
            renderer_.setRenderTarget(coloredTex);
            SDL_RenderCopy(sdlrenderer, text2, nullptr, nullptr);
            SDL_RenderCopy(sdlrenderer, grad, nullptr, nullptr);

            symbols_[i].m_tex = renderer_.createTexture(w1, h1);
            symbols_[i].m_w = w1;
            symbols_[i].m_h = h1;

            TTF_GetGlyphMetrics(font, chid, &symbols_[i].m_minx, &symbols_[i].m_maxx, &symbols_[i].m_miny, &symbols_[i].m_maxy, &symbols_[i].m_advance);
            SDL_SetTextureBlendMode(symbols_[i].m_tex, SDL_BLENDMODE_BLEND);

            renderer_.setRenderTarget(symbols_[i].m_tex);
            SDL_RenderCopy(sdlrenderer, text1, nullptr, nullptr);
            SDL_RenderCopy(sdlrenderer, coloredTex, nullptr, &dst);

            renderer_.setRenderTarget();
            SDL_DestroyTexture(text1);
            SDL_DestroyTexture(text2);
            SDL_DestroyTexture(coloredTex);
        }
        symbolChunks_.push_back(std::move(symbols_));
    }

    SDL_DestroyTexture(grad);
    TTF_CloseFont(font);
    std::cout << generated << " characters generated out of " << charsTotal << ", " << notProvided << " characters not provided\n";
}*/

/*void TextManager::generateOutlinedSymbols(std::vector<std::array<fonts::Symbol, fonts::CHUNK_SIZE>> &symbolChunks_, const fonts::CharChunkDistribution &distrib_, Renderer &renderer_, const std::string &basePath_, const std::string &font_, int size_, int outlineWidth_, const SDL_Color &color_, const SDL_Color &outlineColor_)
{
    std::cout << "Run " << __func__ << " generator\n";
    TTF_Font *font = TTF_OpenFont((basePath_ + font_).c_str(), size_);
    if (font == nullptr)
    {
            std::cout << "Cannot create font: " << SDL_GetError() << std::endl;
            return;
    }

    int notProvided = 0;
    int charsTotal = 0;
    int generated = 0;

    auto *sdlrenderer = renderer_.getRenderer();

    for (const auto &chunkInitVal : distrib_.m_chunks)
    {
        std::array<fonts::Symbol, fonts::CHUNK_SIZE> symbols_;
        for (auto i = 0; i < fonts::CHUNK_SIZE; ++i)
        {
            charsTotal++;
            symbols_[i].m_tex = nullptr;
            uint32_t chid_8 = chunkInitVal * fonts::CHUNK_SIZE + i;
            uint32_t chid = utf8::u8tou32(chid_8, utf8::readCharSize(chid_8));

            if (!TTF_FontHasGlyph(font, chid))
            {
                notProvided++;
                continue;
            }

            TTF_SetFontOutline(font, outlineWidth_);

            // Outline (background)
            auto surf = TTF_RenderGlyph_Solid(font, chid, outlineColor_);
            if (surf == nullptr)
            {
                std::cout << "Cannot create 1st surface: " << SDL_GetError() << std::endl;
                continue;
            }
            auto *text1 = SDL_CreateTextureFromSurface(sdlrenderer, surf);
            SDL_DestroySurface(surf);

            // Inner letter
            TTF_SetFontOutline(font, 0);
            surf = TTF_RenderGlyph_Solid(font, chid, {255, 255, 255, 255});
            if (surf == nullptr)
            {
                std::cout << "Cannot create 2nd surface: " << SDL_GetError() << std::endl;
                continue;
            }
            else
                generated++;
            auto *text2 = SDL_CreateTextureFromSurface(sdlrenderer, surf);
            SDL_DestroySurface(surf);

            // Outline size
            int w1, h1;
            SDL_QueryTexture(text1, NULL, NULL, &w1, &h1);

            // Inner size
            int w2, h2;
            SDL_QueryTexture(text2, NULL, NULL, &w2, &h2);
            SDL_Rect dst;
            dst.x = outlineWidth_;
            dst.y = outlineWidth_;
            dst.w = w2;
            dst.h = h2;


            symbols_[i].m_tex = renderer_.createTexture(w1, h1);
            symbols_[i].m_w = w1;
            symbols_[i].m_h = h1;
            TTF_GetGlyphMetrics(font, chid, &symbols_[i].m_minx, &symbols_[i].m_maxx, &symbols_[i].m_miny, &symbols_[i].m_maxy, &symbols_[i].m_advance);
            SDL_SetTextureBlendMode(symbols_[i].m_tex, SDL_BLENDMODE_BLEND);

            renderer_.setRenderTarget(symbols_[i].m_tex);
            SDL_RenderCopy(sdlrenderer, text1, nullptr, nullptr);
            SDL_RenderCopy(sdlrenderer, text2, nullptr, &dst);

            renderer_.setRenderTarget();
            SDL_DestroyTexture(text1);
            SDL_DestroyTexture(text2);
        }
        symbolChunks_.push_back(std::move(symbols_));
    }

    TTF_CloseFont(font);
    std::cout << generated << " characters generated out of " << charsTotal << ", " << notProvided << " characters not provided\n";
}*/

void TextManager::generateSimpleSymbols(std::vector<std::array<fonts::Symbol, fonts::CHUNK_SIZE>> &symbolChunks_, const fonts::CharChunkDistribution &distrib_, Renderer &renderer_, const std::string &basePath_, const std::string &font_, int size_, const SDL_Color &color_)
{
    std::cout << "Run " << __func__ << " generator" << std::endl;
    TTF_Font *font = TTF_OpenFont((basePath_ + font_).c_str(), size_);
    if (font == nullptr)
    {
            std::cout << "Cannot create font: " << SDL_GetError() << std::endl;
            return;
    }

    int notProvided = 0;
    int charsTotal = 0;
    int generated = 0;

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

            auto surf = TTF_RenderGlyph_Solid(font, chid, color_);
            if (surf == nullptr)
            {
                std::cout << "Cannot create surface: " << SDL_GetError() << std::endl;
                continue;
            }
            else
                generated++;

            auto nsurf = SDL_ConvertSurface(surf, SDL_PIXELFORMAT_ABGR8888);

            symbols_[i].m_tex.m_size.x = nsurf->w;
            symbols_[i].m_tex.m_size.y = nsurf->h;
            symbols_[i].m_tex.m_id = renderer_.surfaceToTexture(nsurf);

            SDL_DestroySurface(surf);
            SDL_DestroySurface(nsurf);

            TTF_GetGlyphMetrics(font, chid, &symbols_[i].m_minx, &symbols_[i].m_maxx, &symbols_[i].m_miny, &symbols_[i].m_maxy, &symbols_[i].m_advance);
        }

        symbolChunks_.push_back(std::move(symbols_));
    }

    TTF_CloseFont(font);
    std::cout << generated << " characters generated out of " << charsTotal << ", " << notProvided << " characters not provided" << std::endl;
}

void TextManager::generateSimpleShadedSymbols(std::vector<std::array<fonts::Symbol, fonts::CHUNK_SIZE>> &symbolChunks_, const fonts::CharChunkDistribution &distrib_, Renderer &renderer_, const std::string &basePath_, const std::string &font_, int size_, const SDL_Color &color_, const SDL_Color &shadeColor_)
{
    std::cout << "Run " << __func__ << " generator" << std::endl;
    TTF_Font *font = TTF_OpenFont((basePath_ + font_).c_str(), size_);
    if (font == nullptr)
    {
            std::cout << "Cannot create font: " << SDL_GetError() << std::endl;
            return;
    }

    int notProvided = 0;
    int charsTotal = 0;
    int generated = 0;

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


            unsigned int unshadedTex = 0;
            unsigned int shadedTex = 0;
            Vector2<int> unshadedSize, shadedSize;

            {
                // Outline (background)
                TTF_SetFontOutline(font, 1);
                auto surf = TTF_RenderGlyph_Solid(font, chid, shadeColor_);
                if (surf == nullptr)
                {
                    std::cout << "Cannot create 1st surface: " << SDL_GetError() << std::endl;
                    continue;
                }
                auto nsurf = SDL_ConvertSurface(surf, SDL_PIXELFORMAT_ABGR8888);
                shadedTex = renderer_.surfaceToTexture(nsurf);
                shadedSize = {nsurf->w, nsurf->h};
                SDL_DestroySurface(surf);
                SDL_DestroySurface(nsurf);

                // Inner letter
                TTF_SetFontOutline(font, 0);
                surf = TTF_RenderGlyph_Solid(font, chid, color_);
                if (surf == nullptr)
                {
                    std::cout << "Cannot create 2nd surface: " << SDL_GetError() << std::endl;
                    continue;
                }
                else
                    generated++;
                nsurf = SDL_ConvertSurface(surf, SDL_PIXELFORMAT_ABGR8888);
                unshadedTex = renderer_.surfaceToTexture(nsurf);
                unshadedSize = {nsurf->w, nsurf->h};
                SDL_DestroySurface(surf);
                SDL_DestroySurface(nsurf);
            }

            symbols_[i].m_tex.m_size.x = shadedSize.x + 1;
            symbols_[i].m_tex.m_size.y = shadedSize.y + 1;
            symbols_[i].m_tex.m_id = renderer_.createTextureRGBA(symbols_[i].m_tex.m_size.x, symbols_[i].m_tex.m_size.y);
            TTF_GetGlyphMetrics(font, chid, &symbols_[i].m_minx, &symbols_[i].m_maxx, &symbols_[i].m_miny, &symbols_[i].m_maxy, &symbols_[i].m_advance);

            renderer_.attachTex(symbols_[i].m_tex.m_id, symbols_[i].m_tex.m_size);
            renderer_.fillRenderer(SDL_Color{255, 255, 255, 0});
            renderer_.renderTexture(shadedTex, {0, 0}, shadedSize, SDL_FLIP_VERTICAL, 1.0f);
            renderer_.renderTexture(unshadedTex, {0, 2}, unshadedSize, SDL_FLIP_VERTICAL, 1.0f);

            renderer_.attachTex();
            dumpErrors();

            glDeleteTextures(1, &unshadedTex);
            glDeleteTextures(1, &shadedTex);
        }
        symbolChunks_.push_back(std::move(symbols_));
    }

    TTF_CloseFont(font);
    std::cout << generated << " characters generated out of " << charsTotal << ", " << notProvided << " characters not provided" << std::endl;
}

void TextManager::renderText(const std::string &text_, int fontid_, Vector2<int> pos_, fonts::HOR_ALIGN horAlign_, Camera *cam_)
{
    U8Wrapper wrp(text_);
    auto begin = wrp.begin();
    uint32_t ch1 = begin.getu8();
    if (horAlign_ != fonts::HOR_ALIGN::LEFT)
    {
        auto len = m_fonts[fontid_][ch1].m_minx;
        for (auto &ch : wrp)
        {
            auto &sym = m_fonts[fontid_][ch.getu8()];
            len += sym.m_advance;
        }

        if (horAlign_ == fonts::HOR_ALIGN::CENTER)
        {
            pos_.x -= len / 2;
        }
        else if (horAlign_ == fonts::HOR_ALIGN::RIGHT)
        {
            pos_.x -= len;
        }
    }

    pos_.x += m_fonts[fontid_][ch1].m_minx;
    for (auto &ch : wrp)
    {
        auto &sym = m_fonts[fontid_][ch.getu8()];
        if (cam_)
            m_renderer.renderTexture(sym.m_tex.m_id, Vector2(pos_.x, pos_.y), sym.m_tex.m_size, SDL_FLIP_NONE, 1.0f, *cam_);
        else
            m_renderer.renderTexture(sym.m_tex.m_id, Vector2(pos_.x, pos_.y), sym.m_tex.m_size, SDL_FLIP_NONE, 1.0f);
        pos_.x += sym.m_advance;
    }
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
