#include "TextManager.h"

fonts::Symbol& fonts::Symbol::operator=(fonts::Symbol &&rhs_)
{
    m_minx = rhs_.m_minx;
    m_maxx = rhs_.m_maxx;
    m_miny = rhs_.m_miny;
    m_maxy = rhs_.m_maxy;
    m_advance = rhs_.m_advance;
    
    m_tex = rhs_.m_tex;
    rhs_.m_tex = nullptr;

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
}

fonts::Symbol::~Symbol()
{
    SDL_DestroyTexture(m_tex);
}

template<typename Func, typename... Args>
fonts::Font::Font(Func generateSymbols_, Args&&... args_)
{
    static_assert(std::is_invocable_v<Func, decltype(m_symbols)&, Args...>, "Function Func should be invokable with these arguments");
    generateSymbols_(m_symbols, std::forward<Args>(args_)...);
}

const fonts::Symbol &fonts::Font::operator[](const char ch_)
{
    return m_symbols[ch_];
}

TextManager::TextManager(Renderer *renderer_, const std::string &basePath_) :
    m_renderer(renderer_),
    m_fonts{fonts::Font(generateSimpleShadedSymbols, *renderer_, basePath_, "/Resources/Fonts/Silkscreen.ttf",  16, SDL_Color{255, 255, 255, 255}, SDL_Color{100, 100, 100, 255})}
{
}


void TextManager::generateOutlinedTexturedSymbols(std::array<fonts::Symbol, 256> &symbols_, Renderer &renderer_, const std::string &basePath_, const std::string &font_, const std::string &texture_, int size_, int outlineWidth_, const SDL_Color &outlineColor_)
{
    std::cout << "Run generator\n";
    TTF_Font *font = TTF_OpenFont((basePath_ + font_).c_str(), size_);
    if (font == nullptr)
    {
            std::cout << "Cannot create font: " << TTF_GetError() << std::endl;
            return;
    }

    auto *grad = renderer_.loadTexture((basePath_ + texture_).c_str());
    SDL_SetTextureBlendMode(grad, SDL_BLENDMODE_MUL);
    auto *sdlrenderer = renderer_.getRenderer();

    for (int i = 0; i < symbols_.size(); ++i)
    {
        TTF_SetFontOutline(font, outlineWidth_);

        // Outline (background)
        auto surf = TTF_RenderGlyph32_Blended(font, Uint32(i), outlineColor_);
        if (surf == nullptr)
            std::cout << "Cannot create 1st surface: " << TTF_GetError() << std::endl;
        auto *text1 = SDL_CreateTextureFromSurface(sdlrenderer, surf);
        SDL_FreeSurface(surf);

        // Inner letter
        TTF_SetFontOutline(font, 0);
        surf = TTF_RenderGlyph32_Blended(font, Uint32(i), {255, 255, 255, 255});
        if (surf == nullptr)
            std::cout << "Cannot create 2nd surface: " << TTF_GetError() << std::endl;
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
        
        TTF_GlyphMetrics32(font, Uint32(i), &symbols_[i].m_minx, &symbols_[i].m_maxx, &symbols_[i].m_miny, &symbols_[i].m_maxy, &symbols_[i].m_advance);
        SDL_SetTextureBlendMode(symbols_[i].m_tex, SDL_BLENDMODE_BLEND);

        renderer_.setRenderTarget(symbols_[i].m_tex);
        SDL_RenderCopy(sdlrenderer, text1, nullptr, nullptr);
        SDL_RenderCopy(sdlrenderer, coloredTex, nullptr, &dst);

        renderer_.setRenderTarget();
        SDL_DestroyTexture(text1);
        SDL_DestroyTexture(text2);
        SDL_DestroyTexture(coloredTex);
    }

    SDL_DestroyTexture(grad);
    TTF_CloseFont(font);
}

void TextManager::generateOutlinedSymbols(std::array<fonts::Symbol, 256> &symbols_, Renderer &renderer_, const std::string &basePath_, const std::string &font_, int size_, int outlineWidth_, const SDL_Color &color_, const SDL_Color &outlineColor_)
{
    std::cout << "Run generator\n";
    TTF_Font *font = TTF_OpenFont((basePath_ + font_).c_str(), size_);
    if (font == nullptr)
    {
            std::cout << "Cannot create font: " << TTF_GetError() << std::endl;
            return;
    }

    auto *sdlrenderer = renderer_.getRenderer();

    for (int i = 0; i < symbols_.size(); ++i)
    {
        TTF_SetFontOutline(font, outlineWidth_);

        // Outline (background)
        auto surf = TTF_RenderGlyph32_Solid(font, Uint32(i), outlineColor_);
        if (surf == nullptr)
            std::cout << "Cannot create 1st surface: " << TTF_GetError() << std::endl;
        auto *text1 = SDL_CreateTextureFromSurface(sdlrenderer, surf);
        SDL_FreeSurface(surf);

        // Inner letter
        TTF_SetFontOutline(font, 0);
        surf = TTF_RenderGlyph32_Solid(font, Uint32(i), {255, 255, 255, 255});
        if (surf == nullptr)
            std::cout << "Cannot create 2nd surface: " << TTF_GetError() << std::endl;
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
        TTF_GlyphMetrics32(font, Uint32(i), &symbols_[i].m_minx, &symbols_[i].m_maxx, &symbols_[i].m_miny, &symbols_[i].m_maxy, &symbols_[i].m_advance);
        SDL_SetTextureBlendMode(symbols_[i].m_tex, SDL_BLENDMODE_BLEND);

        renderer_.setRenderTarget(symbols_[i].m_tex);
        SDL_RenderCopy(sdlrenderer, text1, nullptr, nullptr);
        SDL_RenderCopy(sdlrenderer, text2, nullptr, &dst);

        renderer_.setRenderTarget();
        SDL_DestroyTexture(text1);
        SDL_DestroyTexture(text2);
    }

    TTF_CloseFont(font);
}

void TextManager::generateSimpleSymbols(std::array<fonts::Symbol, 256> &symbols_, Renderer &renderer_, const std::string &basePath_, const std::string &font_, int size_, const SDL_Color &color_)
{
    std::cout << "Run generator\n";
    TTF_Font *font = TTF_OpenFont((basePath_ + font_).c_str(), size_);
    if (font == nullptr)
    {
            std::cout << "Cannot create font: " << TTF_GetError() << std::endl;
            return;
    }

    auto *sdlrenderer = renderer_.getRenderer();

    for (int i = 0; i < symbols_.size(); ++i)
    {
        auto surf = TTF_RenderGlyph32_Solid(font, Uint32(i), color_);
        if (surf == nullptr)
            std::cout << "Cannot create surface: " << TTF_GetError() << std::endl;
        auto *text = SDL_CreateTextureFromSurface(sdlrenderer, surf);
        SDL_FreeSurface(surf);

        symbols_[i].m_tex = text;
        SDL_QueryTexture(text, nullptr, nullptr, &symbols_[i].m_w, &symbols_[i].m_h);
        TTF_GlyphMetrics32(font, Uint32(i), &symbols_[i].m_minx, &symbols_[i].m_maxx, &symbols_[i].m_miny, &symbols_[i].m_maxy, &symbols_[i].m_advance);
        SDL_SetTextureBlendMode(symbols_[i].m_tex, SDL_BLENDMODE_BLEND);
    }

    TTF_CloseFont(font);
}

void TextManager::generateSimpleShadedSymbols(std::array<fonts::Symbol, 256> &symbols_, Renderer &renderer_, const std::string &basePath_, const std::string &font_, int size_, const SDL_Color &color_, const SDL_Color &shadeColor_)
{
    std::cout << "Run generator\n";
    TTF_Font *font = TTF_OpenFont((basePath_ + font_).c_str(), size_);
    if (font == nullptr)
    {
            std::cout << "Cannot create font: " << TTF_GetError() << std::endl;
            return;
    }

    auto *sdlrenderer = renderer_.getRenderer();

    for (int i = 0; i < symbols_.size(); ++i)
    {
        TTF_SetFontOutline(font, 0.5);

        // Outline (background)
        auto surf = TTF_RenderGlyph32_Solid(font, Uint32(i), shadeColor_);
        if (surf == nullptr)
            std::cout << "Cannot create 1st surface: " << TTF_GetError() << std::endl;
        auto *shade = SDL_CreateTextureFromSurface(sdlrenderer, surf);
        SDL_FreeSurface(surf);

        // Inner letter
        TTF_SetFontOutline(font, 0);
        surf = TTF_RenderGlyph32_Solid(font, Uint32(i), color_);
        if (surf == nullptr)
            std::cout << "Cannot create 2nd surface: " << TTF_GetError() << std::endl;
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
        TTF_GlyphMetrics32(font, Uint32(i), &symbols_[i].m_minx, &symbols_[i].m_maxx, &symbols_[i].m_miny, &symbols_[i].m_maxy, &symbols_[i].m_advance);
        SDL_SetTextureBlendMode(symbols_[i].m_tex, SDL_BLENDMODE_BLEND);

        renderer_.setRenderTarget(symbols_[i].m_tex);
        SDL_RenderCopy(sdlrenderer, shade, nullptr, &dstShadow1);
        SDL_RenderCopy(sdlrenderer, shade, nullptr, &dstShadow2);
        SDL_RenderCopy(sdlrenderer, shade, nullptr, &dstShadow3);
        SDL_RenderCopy(sdlrenderer, letter, nullptr, &dstMain);

        renderer_.setRenderTarget();
        SDL_DestroyTexture(shade);
        SDL_DestroyTexture(letter);
    }

    TTF_CloseFont(font);
}

void TextManager::renderText(const std::string &text, int fontid, Vector2<float> pos, fonts::HOR_ALIGN horAlign_)
{
    if (horAlign_ != fonts::HOR_ALIGN::LEFT)
    {
        float len = m_fonts[fontid][text[0]].m_minx;
        for (auto &ch : text)
        {
            auto &sym = m_fonts[fontid][ch];
            len += sym.m_advance;
        }

        if (horAlign_ == fonts::HOR_ALIGN::CENTER)
        {
            pos.x -= len / 2.0f;
        }
        else if (horAlign_ == fonts::HOR_ALIGN::RIGHT)
        {
            pos.x -= len;
        }
    }

    pos.x += m_fonts[fontid][text[0]].m_minx;
    for (auto &ch : text)
    {
        auto &sym = m_fonts[fontid][ch];
        m_renderer->renderTexture(sym.m_tex, pos.x, pos.y + sym.m_miny, sym.m_w, sym.m_h);
        pos.x += sym.m_advance;
    }
}

SDL_Texture *TextManager::renderStraight(const std::string &s_)
{
    TTF_Font *font = TTF_OpenFont("T:/cppStuff/ActionPlatformer/Resources/Fonts/Silkscreen.ttf", 12);

    auto surf = TTF_RenderUTF8_Solid(font, "Обычный текст", {255, 255, 255, 255});
    if (surf == nullptr)
        std::cout << "Cannot create 1st surface: " << TTF_GetError() << std::endl;
    auto *text = m_renderer->createTextureFromSurface(surf);
    SDL_FreeSurface(surf);
    

    TTF_CloseFont(font);

    return text;
}
