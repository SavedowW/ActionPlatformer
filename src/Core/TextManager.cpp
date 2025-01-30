#include "GameData.h"
#include "TextManager.h"
#include <fstream>
#include "utf8.h"

fonts::Symbol& fonts::Symbol::operator=(fonts::Symbol &&rhs_)
{
    m_minx = rhs_.m_minx;
    m_maxx = rhs_.m_maxx;
    m_miny = rhs_.m_miny;
    m_maxy = rhs_.m_maxy;
    m_advance = rhs_.m_advance;
    
    m_tex = rhs_.m_tex;
    rhs_.m_tex = nullptr;

    m_w = rhs_.m_w;
    m_h = rhs_.m_h;

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
    rhs_.m_tex = nullptr;

    m_w = rhs_.m_w;
    m_h = rhs_.m_h;
}

fonts::Symbol::~Symbol()
{
    SDL_DestroyTexture(m_tex);
}

template<typename Func, typename... Args>
fonts::Font::Font(Func generateSymbols_, int height_, const CharChunkDistribution &distrib_, Args&&... args_) :
    m_distrib(distrib_),
    m_height(height_)
{
    std::cout << "Initializing font...\n";
    static_assert(std::is_invocable_v<Func, decltype(m_symbols)&, decltype(distrib_), Args...>, "Function Func should be invokable with these arguments");
    generateSymbols_(m_symbols, distrib_, std::forward<Args>(args_)...);
}

const fonts::Symbol &fonts::Font::operator[](uint32_t ch_) const
{
    return m_symbols[m_distrib.m_chunkSearch[ch_]][ch_ % CHUNK_SIZE];
}

TextManager::TextManager(Renderer *renderer_, const std::string &basePath_) :
    m_charChunks(basePath_ + "/Resources/GeneralCharacterList.txt"),
    m_renderer(renderer_),
    m_fonts{fonts::Font(generateSimpleShadedSymbols, 12, m_charChunks, *renderer_, basePath_, "/Resources/Fonts/Silkscreen.ttf",  12, SDL_Color{255, 255, 255, 255}, SDL_Color{100, 100, 100, 255}),
    fonts::Font(generateSimpleSymbols, 18, m_charChunks, *renderer_, basePath_, "/Resources/Fonts/Silkscreen.ttf",  18, SDL_Color{255, 255, 255, 255}),
    fonts::Font(generateSimpleSymbols, 8, m_charChunks, *renderer_, basePath_, "/Resources/Fonts/Silkscreen.ttf",  8, SDL_Color{255, 255, 255, 255}),
    fonts::Font(generateSimpleSymbols, 16, m_charChunks, *renderer_, basePath_, "/Resources/Fonts/VanillaExtractRegular.ttf",  16, gamedata::colors::LVL1)}
    //m_fonts{fonts::Font(generateOutlinedSymbols, m_charChunks, *renderer_, basePath_, "/Resources/Fonts/Silkscreen.ttf",  24, 2, SDL_Color{0, 0, 0, 255}, SDL_Color{0, 100, 0, 255})}
    //m_fonts{fonts::Font(generateOutlinedTexturedSymbols, m_charChunks, *renderer_, basePath_, "/Resources/Fonts/Silkscreen.ttf", "/Resources/Fonts/fontBack.png",  24, 2, SDL_Color{0, 0, 0, 255})}
{
}


void TextManager::generateOutlinedTexturedSymbols(std::vector<std::array<fonts::Symbol, fonts::CHUNK_SIZE>> &symbolChunks_, const fonts::CharChunkDistribution &distrib_, Renderer &renderer_, const std::string &basePath_, const std::string &font_, const std::string &texture_, int size_, int outlineWidth_, const SDL_Color &outlineColor_)
{
    std::cout << "Run " << __func__ << " generator\n";
    TTF_Font *font = TTF_OpenFont((basePath_ + font_).c_str(), size_);
    if (font == nullptr)
    {
            std::cout << "Cannot create font: " << TTF_GetError() << std::endl;
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

            if (!TTF_GlyphIsProvided32(font, chid))
            {
                notProvided++;
                continue;
            }

            TTF_SetFontOutline(font, outlineWidth_);

            // Outline (background)
            auto surf = TTF_RenderGlyph32_Solid(font, chid, outlineColor_);
            if (surf == nullptr)
            {
                std::cout << "Cannot create 1st surface: " << TTF_GetError() << std::endl;
                continue;
            }
            auto *text1 = SDL_CreateTextureFromSurface(sdlrenderer, surf);
            SDL_FreeSurface(surf);

            // Inner letter
            TTF_SetFontOutline(font, 0);
            surf = TTF_RenderGlyph32_Solid(font, chid, {255, 255, 255, 255});
            if (surf == nullptr)
            {
                std::cout << "Cannot create 2nd surface: " << TTF_GetError() << std::endl;
                continue;
            }
            else
                generated++;
            auto *text2 = SDL_CreateTextureFromSurface(sdlrenderer, surf);
            SDL_FreeSurface(surf);

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

            TTF_GlyphMetrics32(font, chid, &symbols_[i].m_minx, &symbols_[i].m_maxx, &symbols_[i].m_miny, &symbols_[i].m_maxy, &symbols_[i].m_advance);
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
}

void TextManager::generateOutlinedSymbols(std::vector<std::array<fonts::Symbol, fonts::CHUNK_SIZE>> &symbolChunks_, const fonts::CharChunkDistribution &distrib_, Renderer &renderer_, const std::string &basePath_, const std::string &font_, int size_, int outlineWidth_, const SDL_Color &color_, const SDL_Color &outlineColor_)
{
    std::cout << "Run " << __func__ << " generator\n";
    TTF_Font *font = TTF_OpenFont((basePath_ + font_).c_str(), size_);
    if (font == nullptr)
    {
            std::cout << "Cannot create font: " << TTF_GetError() << std::endl;
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

            if (!TTF_GlyphIsProvided32(font, chid))
            {
                notProvided++;
                continue;
            }

            TTF_SetFontOutline(font, outlineWidth_);

            // Outline (background)
            auto surf = TTF_RenderGlyph32_Solid(font, chid, outlineColor_);
            if (surf == nullptr)
            {
                std::cout << "Cannot create 1st surface: " << TTF_GetError() << std::endl;
                continue;
            }
            auto *text1 = SDL_CreateTextureFromSurface(sdlrenderer, surf);
            SDL_FreeSurface(surf);

            // Inner letter
            TTF_SetFontOutline(font, 0);
            surf = TTF_RenderGlyph32_Solid(font, chid, {255, 255, 255, 255});
            if (surf == nullptr)
            {
                std::cout << "Cannot create 2nd surface: " << TTF_GetError() << std::endl;
                continue;
            }
            else
                generated++;
            auto *text2 = SDL_CreateTextureFromSurface(sdlrenderer, surf);
            SDL_FreeSurface(surf);

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
            TTF_GlyphMetrics32(font, chid, &symbols_[i].m_minx, &symbols_[i].m_maxx, &symbols_[i].m_miny, &symbols_[i].m_maxy, &symbols_[i].m_advance);
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
}

void TextManager::generateSimpleSymbols(std::vector<std::array<fonts::Symbol, fonts::CHUNK_SIZE>> &symbolChunks_, const fonts::CharChunkDistribution &distrib_, Renderer &renderer_, const std::string &basePath_, const std::string &font_, int size_, const SDL_Color &color_)
{
    std::cout << "Run " << __func__ << " generator\n";
    TTF_Font *font = TTF_OpenFont((basePath_ + font_).c_str(), size_);
    if (font == nullptr)
    {
            std::cout << "Cannot create font: " << TTF_GetError() << std::endl;
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

            if (!TTF_GlyphIsProvided32(font, chid))
            {
                notProvided++;
                continue;
            }

            auto surf = TTF_RenderGlyph32_Solid(font, chid, color_);
            if (surf == nullptr)
            {
                std::cout << "Cannot create surface: " << TTF_GetError() << std::endl;
                continue;
            }
            else
                generated++;
            
            auto *text = SDL_CreateTextureFromSurface(sdlrenderer, surf);
            SDL_FreeSurface(surf);

            symbols_[i].m_tex = text;
            SDL_QueryTexture(text, nullptr, nullptr, &symbols_[i].m_w, &symbols_[i].m_h);
            TTF_GlyphMetrics32(font, chid, &symbols_[i].m_minx, &symbols_[i].m_maxx, &symbols_[i].m_miny, &symbols_[i].m_maxy, &symbols_[i].m_advance);
            SDL_SetTextureBlendMode(symbols_[i].m_tex, SDL_BLENDMODE_BLEND);
        }

        symbolChunks_.push_back(std::move(symbols_));
    }

    TTF_CloseFont(font);
    std::cout << generated << " characters generated out of " << charsTotal << ", " << notProvided << " characters not provided\n";
}

void TextManager::generateSimpleShadedSymbols(std::vector<std::array<fonts::Symbol, fonts::CHUNK_SIZE>> &symbolChunks_, const fonts::CharChunkDistribution &distrib_, Renderer &renderer_, const std::string &basePath_, const std::string &font_, int size_, const SDL_Color &color_, const SDL_Color &shadeColor_)
{
    std::cout << "Run " << __func__ << " generator\n";
    TTF_Font *font = TTF_OpenFont((basePath_ + font_).c_str(), size_);
    if (font == nullptr)
    {
            std::cout << "Cannot create font: " << TTF_GetError() << std::endl;
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
            uint32_t chid = utf8::u8tou32(chid_8, utf8::readCharSize(chid_8)); // TODO: update in main project

            if (!TTF_GlyphIsProvided32(font, chid))
            {
                notProvided++;
                continue;
            }

            TTF_SetFontOutline(font, 1);

            // Outline (background)
            auto surf = TTF_RenderGlyph32_Solid(font, chid, shadeColor_);
            if (surf == nullptr)
            {
                std::cout << "Cannot create 1st surface: " << TTF_GetError() << std::endl;
                continue;
            }
            auto *shade = SDL_CreateTextureFromSurface(sdlrenderer, surf);
            SDL_FreeSurface(surf);

            // Inner letter
            TTF_SetFontOutline(font, 0);
            surf = TTF_RenderGlyph32_Solid(font, chid, color_);
            if (surf == nullptr)
            {
                std::cout << "Cannot create 2nd surface: " << TTF_GetError() << std::endl;
                continue;
            }
            else
                generated++;
            auto *letter = SDL_CreateTextureFromSurface(sdlrenderer, surf);
            SDL_FreeSurface(surf);

            // Inner size
            int shadeW, shadeH;
            int innerW, innerH;
            SDL_QueryTexture(shade, NULL, NULL, &shadeW, &shadeH);
            SDL_QueryTexture(letter, NULL, NULL, &innerW, &innerH);

            SDL_Rect dstShadow1;
            dstShadow1.x = 1;
            dstShadow1.y = 1;
            dstShadow1.w = shadeW;
            dstShadow1.h = shadeH;

            SDL_Rect dstShadow2;
            dstShadow2.x = 1;
            dstShadow2.y = 0;
            dstShadow2.w = shadeW;
            dstShadow2.h = shadeH;

            SDL_Rect dstShadow3;
            dstShadow3.x = 0;
            dstShadow3.y = 1;
            dstShadow3.w = shadeW;
            dstShadow3.h = shadeH;

            SDL_Rect dstMain;
            dstMain.x = 0;
            dstMain.y = 0;
            dstMain.w = innerW;
            dstMain.h = innerH;


            symbols_[i].m_w = shadeW + 1;
            symbols_[i].m_h = shadeH + 1;
            symbols_[i].m_tex = renderer_.createTexture(symbols_[i].m_w, symbols_[i].m_h);
            TTF_GlyphMetrics32(font, chid, &symbols_[i].m_minx, &symbols_[i].m_maxx, &symbols_[i].m_miny, &symbols_[i].m_maxy, &symbols_[i].m_advance);

            renderer_.setRenderTarget(symbols_[i].m_tex);
            SDL_RenderCopy(sdlrenderer, shade, nullptr, &dstShadow1);
            //SDL_RenderCopy(sdlrenderer, shade, nullptr, &dstShadow2);
            //SDL_RenderCopy(sdlrenderer, shade, nullptr, &dstShadow3);
            SDL_RenderCopy(sdlrenderer, letter, nullptr, &dstMain);

            renderer_.setRenderTarget();
            SDL_DestroyTexture(shade);
            SDL_DestroyTexture(letter);
        }
        symbolChunks_.push_back(std::move(symbols_));
    }

    TTF_CloseFont(font);
    std::cout << generated << " characters generated out of " << charsTotal << ", " << notProvided << " characters not provided\n";
}

void TextManager::renderText(const std::string &text_, int fontid_, Vector2<float> pos_, fonts::HOR_ALIGN horAlign_, Camera *cam_)
{
    U8Wrapper wrp(text_);
    auto begin = wrp.begin();
    uint32_t ch1 = begin.getu8();
    if (horAlign_ != fonts::HOR_ALIGN::LEFT)
    {
        float len = m_fonts[fontid_][ch1].m_minx;
        for (auto &ch : wrp)
        {
            auto &sym = m_fonts[fontid_][ch.getu8()];
            len += sym.m_advance;
        }

        if (horAlign_ == fonts::HOR_ALIGN::CENTER)
        {
            pos_.x -= len / 2.0f;
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
            m_renderer->renderTexture(sym.m_tex, pos_.x, pos_.y, sym.m_w, sym.m_h, *cam_, 0, SDL_FLIP_NONE);
        else
            m_renderer->renderTexture(sym.m_tex, pos_.x, pos_.y, (float)sym.m_w, (float)sym.m_h);
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
        throw std::string("Failed to open charlist: \"") + charlist_ + "\"";

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

    std::cout << m_chunks.size() << " chunks, " << m_chunks.size() * CHUNK_SIZE << " characters total\n";

    int i = 0;
    for (auto &el : m_chunks)
        m_chunkSearch.addPropertyValue(el * CHUNK_SIZE, i++);
}
