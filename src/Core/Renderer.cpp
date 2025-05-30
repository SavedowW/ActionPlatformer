#include "Renderer.h"
#include <stdexcept>
#include "GameData.h"
#include "TextureManager.h"

Renderer::Renderer(Window &window_)
{
    auto win = window_.getWindow();
    if (!win)
        throw std::runtime_error("Trying to create renderer without window");

    m_renderer = SDL_CreateRenderer(window_.getWindow(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    if (m_renderer == nullptr) {
        std::cout << "Renderer creation error: " << SDL_GetError() << std::endl;
        throw std::runtime_error("Cannot create renderer");
    }

    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

	SDL_RenderSetLogicalSize(m_renderer, gamedata::global::maxCameraSize.x, gamedata::global::maxCameraSize.y);

	auto *tex = createTexture(gamedata::global::maxCameraSize);
	SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
	m_backbuffGameplay.setTexture(tex);

	tex = createTexture(gamedata::global::hudLayerResolution);
	SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
	m_backbuffHUD.setTexture(tex);

}

Renderer::~Renderer()
{
    SDL_DestroyRenderer(m_renderer);
}

Renderer& Renderer::operator=(Renderer &&rhs)
{
    m_renderer = rhs.m_renderer;

    rhs.m_renderer = nullptr;

    return *this;
}

Renderer::Renderer(Renderer &&rhs)
{
    m_renderer = rhs.m_renderer;

    rhs.m_renderer = nullptr;
}

SDL_Texture* Renderer::createTexture(int w_, int h_)
{
	auto *tex = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, w_, h_);
	SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
	return tex;
}

SDL_Texture* Renderer::createTexture(const Vector2<int>& size_)
{
	auto *tex = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, size_.x, size_.y);
	SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
	return tex;
}

SDL_Texture* Renderer::loadTexture(const char* file_)
{
    SDL_Texture* texture = IMG_LoadTexture(m_renderer, file_);
    if (!texture) {
        std::cout << "Texture loading problem: " << file_ << "  |  " << IMG_GetError() << std::endl;
    }
    return texture;
}

SDL_Texture* Renderer::createTextureFromSurface(SDL_Surface* sur_)
{
    return SDL_CreateTextureFromSurface(m_renderer, sur_);
}

void Renderer::renderTexture(SDL_Texture* tex_, int x_, int y_, int w_, int h_, float angle_, SDL_Point* center_, SDL_RendererFlip flip_)
{
    SDL_Rect dst;

    dst.x = x_;
    dst.y = y_;
    dst.w = w_;
    dst.h = h_;

    SDL_RenderCopyEx(m_renderer, tex_, NULL, &dst, angle_, center_, flip_);
}

void Renderer::renderTexture(SDL_Texture *tex_, int x_, int y_, int w_, int h_, const Camera &cam_, float angle_, const Vector2<int> &center_, SDL_RendererFlip flip_)
{
    Vector2<int> camTL = Vector2<int>(cam_.getPos()) - Vector2{m_backbuffGameplay.m_w, m_backbuffGameplay.m_h} / 2;
	Vector2<int> texTL = Vector2{x_, y_} - camTL;

	SDL_Point center;
	center.x = center_.x;
	center.y = center_.y;
	
	renderTexture(tex_, texTL.x, texTL.y, w_, h_, angle_, &center, flip_);
}

void Renderer::renderTexture(SDL_Texture* tex_, const SDL_Rect *src_, const SDL_Rect *dst_, float angle_, SDL_Point* center_, SDL_RendererFlip flip_)
{
    SDL_RenderCopyEx(m_renderer, tex_, src_, dst_, angle_, center_, flip_);
}

void Renderer::renderTexture(SDL_Texture *tex_, int x_, int y_, int w_, int h_, const SDL_Rect &src, const Camera &cam_, SDL_RendererFlip flip_)
{
    Vector2<int> camTL = Vector2<int>(cam_.getPos()) - Vector2{m_backbuffGameplay.m_w, m_backbuffGameplay.m_h} / 2;
	Vector2<int> texTL = Vector2{x_, y_} - camTL;

    SDL_Rect dst;
    dst.x = texTL.x;
    dst.y = texTL.y;
    dst.w = w_;
    dst.h = h_;

    SDL_RenderCopyEx(m_renderer, tex_, &src, &dst, 0, nullptr, flip_);
}

void Renderer::renderTexture(SDL_Texture* tex_, int x_, int y_, int w_, int h_, const Camera &cam_, float angle_, SDL_RendererFlip flip_)
{
	Vector2<int> camTL = Vector2<int>(cam_.getPos()) - Vector2{m_backbuffGameplay.m_w, m_backbuffGameplay.m_h} / 2;
	Vector2<int> texTL = Vector2{x_, y_} - camTL;

	SDL_Point center;
	center.x = w_ / 2;
	center.y = h_ / 2;
	
	renderTexture(tex_, texTL.x, texTL.y, w_, h_, angle_, &center, flip_);
}

void Renderer::renderTexture(SDL_Texture* tex_, int x_, int y_, int w_, int h_)
{
    SDL_Rect dst;

    dst.x = x_;
    dst.y = y_;
    dst.w = w_;
    dst.h = h_;

    SDL_RenderCopy(m_renderer, tex_, NULL, &dst);
}

void Renderer::drawLine(const Vector2<int> &p1_, const Vector2<int> &p2_, const SDL_Color &col_, const Camera &cam_)
{
    Vector2<int> camTL = Vector2<int>(cam_.getPos()) - Vector2{m_backbuffGameplay.m_w, m_backbuffGameplay.m_h} / 2;
    Vector2<int> p1 = p1_ - camTL;
    Vector2<int> p2 = p2_ - camTL;

    SDL_SetRenderDrawColor(m_renderer, col_.r, col_.g, col_.b, col_.a);
    SDL_RenderDrawLine(m_renderer, p1.x, p1.y, p2.x, p2.y);
}

void Renderer::drawRectangle(const Vector2<int> &pos_, const Vector2<int> &size_, const SDL_Color &col_, const Camera &cam_)
{
	Vector2<int> camTL = Vector2<int>(cam_.getPos()) - Vector2{m_backbuffGameplay.m_w, m_backbuffGameplay.m_h} / 2;
	Vector2<int> rectTL = pos_ - camTL;

	SDL_SetRenderDrawColor(m_renderer, col_.r, col_.g, col_.b, col_.a);
	SDL_Rect rect = { rectTL.x, rectTL.y, size_.x, size_.y };
	SDL_RenderDrawRect(m_renderer, &rect);
}

void Renderer::drawRectangle(const Vector2<int> &pos_, const Vector2<int> &size_, const SDL_Color& col_)
{
    SDL_SetRenderDrawColor(m_renderer, col_.r, col_.g, col_.b, col_.a);
    SDL_Rect rect = { pos_.x, pos_.y, size_.x, size_.y };
    SDL_RenderDrawRect(m_renderer, &rect);
}

void Renderer::fillRectangle(const Vector2<int> &pos_, const Vector2<int> &size_, const SDL_Color& col_, const Camera &cam_)
{
	Vector2<int> camTL = Vector2<int>(cam_.getPos()) - Vector2{m_backbuffGameplay.m_w, m_backbuffGameplay.m_h} / 2;
	Vector2<int> rectTL = pos_ - camTL;

	SDL_SetRenderDrawColor(m_renderer, col_.r, col_.g, col_.b, col_.a);
	SDL_Rect rect = { rectTL.x, rectTL.y, static_cast<int>(size_.x), static_cast<int>(size_.y) };
	SDL_RenderFillRect(m_renderer, &rect);
}

void Renderer::fillRectangle(const Vector2<int> &pos_, const Vector2<int> &size_, const SDL_Color& col_)
{
    SDL_SetRenderDrawColor(m_renderer, col_.r, col_.g, col_.b, col_.a);
    SDL_Rect rect = { pos_.x, pos_.y, size_.x, size_.y };
    SDL_RenderFillRect(m_renderer, &rect);
}

void Renderer::drawCross(const Vector2<int> &center_, const Vector2<int> &vSize_, const Vector2<int> &hSize_, const SDL_Color &col_, const Camera &cam_)
{
    Vector2<int> camTL = Vector2<int>(cam_.getPos()) - Vector2{m_backbuffGameplay.m_w, m_backbuffGameplay.m_h} / 2;
	Vector2<int> center = center_ - camTL;

	SDL_SetRenderDrawColor(m_renderer, col_.r, col_.g, col_.b, col_.a);
	SDL_Rect vRect[2] = {{ center.x - vSize_.x / 2, center.y - vSize_.y / 2, vSize_.x, vSize_.y },
	                { center.x - hSize_.x / 2, center.y - hSize_.y / 2, hSize_.x, hSize_.y }};
	SDL_RenderDrawRects(m_renderer, vRect, 2);
}

void Renderer::drawCross(const Vector2<int> &center_, const Vector2<int> &vSize_, const Vector2<int> &hSize_, const SDL_Color &col_)
{
	SDL_SetRenderDrawColor(m_renderer, col_.r, col_.g, col_.b, col_.a);
	SDL_Rect vRect[2] = {{ center_.x - vSize_.x / 2, center_.y - vSize_.y / 2, vSize_.x, vSize_.y },
	                { center_.x - hSize_.x / 2, center_.y - hSize_.y / 2, hSize_.x, hSize_.y }};
	SDL_RenderDrawRects(m_renderer, vRect, 2);
}

void Renderer::drawGeometry(SDL_Texture *texture, const SDL_Vertex *vertices, int num_vertices, const int *indices, int num_indices)
{
    SDL_ClearError();
    if (SDL_RenderGeometry(m_renderer, texture, vertices, num_vertices, indices, num_indices))
    {
        std::cout << "Failed to render geometry: " << SDL_GetError() << std::endl;
    }
}

void Renderer::drawCollider(const Collider &cld_, const SDL_Color &fillCol_, const Camera &cam_)
{
	Vector2<int> camTL = Vector2<int>(cam_.getPos()) - Vector2{m_backbuffGameplay.m_w, m_backbuffGameplay.m_h} / 2;
	Vector2<int> rectTL = cld_.m_topLeft - camTL;

	SDL_Rect rect = { rectTL.x, rectTL.y, cld_.m_size.x, cld_.m_size.y };
	SDL_SetRenderDrawColor(m_renderer, fillCol_.r, fillCol_.g, fillCol_.b, fillCol_.a);
	SDL_RenderFillRect(m_renderer, &rect);
}

void Renderer::drawCollider(const SlopeCollider &cld_, const SDL_Color &fillCol_, const Camera &cam_)
{
    SDL_SetRenderDrawColor(m_renderer, fillCol_.r, fillCol_.g, fillCol_.b, fillCol_.a);
    
    SDL_Vertex vxes[4];
    int idces[6] = {0, 1, 2, 2, 3, 0};

    Vector2<int> camTL = Vector2<int>(cam_.getPos()) - Vector2{m_backbuffGameplay.m_w, m_backbuffGameplay.m_h} / 2;

    for (int i = 0; i < 4; ++i)
    {
        Vector2<int> pos = cld_.m_points[i] - camTL;
        vxes[i].position.x = pos.x;
        vxes[i].position.y = pos.y;
        vxes[i].color = fillCol_;
    }
    vxes[1].position.x += 1;
    vxes[2].position.x += 1;
    vxes[2].position.y += 1;
    vxes[3].position.y += 1;

    if (SDL_RenderGeometry(m_renderer, nullptr, vxes, 4, idces, 6))
    {
        std::cout << "Failed to render geometry: " << SDL_GetError() << std::endl;
    }
}

void Renderer::drawCollider(const Collider &cld_, const SDL_Color &fillCol_, const SDL_Color &borderCol_, const Camera &cam_)
{
    Vector2<int> camTL = Vector2<int>(cam_.getPos()) - Vector2{m_backbuffGameplay.m_w, m_backbuffGameplay.m_h} / 2;
	Vector2<int> rectTL = cld_.m_topLeft - camTL;

	SDL_Rect rect = { rectTL.x, rectTL.y, cld_.m_size.x, cld_.m_size.y };
	SDL_SetRenderDrawColor(m_renderer, fillCol_.r, fillCol_.g, fillCol_.b, fillCol_.a);
	SDL_RenderFillRect(m_renderer, &rect);
    SDL_SetRenderDrawColor(m_renderer, borderCol_.r, borderCol_.g, borderCol_.b, borderCol_.a);
	SDL_RenderDrawRect(m_renderer, &rect);
}

void Renderer::drawCollider(const SlopeCollider &cld_, const SDL_Color &fillCol_, const SDL_Color &borderCol_, const Camera &cam_)
{
    SDL_SetRenderDrawColor(m_renderer, fillCol_.r, fillCol_.g, fillCol_.b, fillCol_.a);
    
    SDL_Vertex vxes[4];
    int idces[6] = {0, 1, 2, 2, 3, 0};

    Vector2<int> camTL = Vector2<int>(cam_.getPos()) - Vector2{m_backbuffGameplay.m_w, m_backbuffGameplay.m_h} / 2;

    for (int i = 0; i < 4; ++i)
    {
        Vector2<int> pos = cld_.m_points[i] - camTL;
        vxes[i].position.x = pos.x;
        vxes[i].position.y = pos.y;
        vxes[i].color = fillCol_;
    }
    vxes[1].position.x += 1;
    vxes[2].position.x += 1;
    vxes[2].position.y += 1;
    vxes[3].position.y += 1;

    if (SDL_RenderGeometry(m_renderer, nullptr, vxes, 4, idces, 6))
    {
        std::cout << "Failed to render geometry: " << SDL_GetError() << std::endl;
    }
    
    SDL_SetRenderDrawColor(m_renderer, borderCol_.r, borderCol_.g, borderCol_.b, borderCol_.a);

    SDL_RenderDrawLine(m_renderer, vxes[0].position.x, vxes[0].position.y, vxes[1].position.x, vxes[1].position.y);
    SDL_RenderDrawLine(m_renderer, vxes[1].position.x, vxes[1].position.y, vxes[2].position.x, vxes[2].position.y);
    SDL_RenderDrawLine(m_renderer, vxes[2].position.x, vxes[2].position.y, vxes[3].position.x, vxes[3].position.y);
    SDL_RenderDrawLine(m_renderer, vxes[3].position.x, vxes[3].position.y, vxes[0].position.x, vxes[0].position.y);
}

void Renderer::prepareRenderer(const SDL_Color& col_)
{
	SDL_RenderSetLogicalSize(m_renderer, m_backbuffGameplay.m_w, m_backbuffGameplay.m_h);
	SDL_SetRenderTarget(m_renderer, m_backbuffGameplay.getSprite());
	SDL_SetRenderDrawColor(m_renderer, col_.r, col_.g, col_.b, col_.a);
	SDL_RenderClear(m_renderer);
}

void Renderer::switchToHUD(const SDL_Color &col_)
{
	SDL_RenderSetLogicalSize(m_renderer, m_backbuffHUD.m_w, m_backbuffHUD.m_h);
	SDL_SetRenderTarget(m_renderer, m_backbuffHUD.getSprite());
	SDL_SetRenderDrawColor(m_renderer, col_.r, col_.g, col_.b, col_.a);
	SDL_RenderClear(m_renderer);
}

void Renderer::fillRenderer(const SDL_Color& col_)
{
	SDL_SetRenderDrawColor(m_renderer, col_.r, col_.g, col_.b, col_.a);
	SDL_RenderClear(m_renderer);
}

void Renderer::updateScreen(const Camera &cam_)
{
	// Render gameplay
	float realScaling = cam_.getScale() / gamedata::global::maxCameraScale;
	auto targetScale = gamedata::global::defaultWindowResolution / realScaling;
	auto delta = targetScale - gamedata::global::defaultWindowResolution;
	auto TL = delta / (-2.0f);
	SDL_FRect dst;
	dst.x = TL.x;
	dst.y = TL.y;
	dst.w = targetScale.x;
	dst.h = targetScale.y;

	SDL_SetRenderTarget(m_renderer, nullptr);
	SDL_RenderSetLogicalSize(m_renderer, gamedata::global::defaultWindowResolution.x, gamedata::global::defaultWindowResolution.y);
	SDL_RenderCopyF(m_renderer, m_backbuffGameplay.getSprite(), nullptr, &dst);

	// Render HUD
	SDL_RenderCopyF(m_renderer, m_backbuffHUD.getSprite(), nullptr, nullptr);

	SDL_RenderPresent(m_renderer);
}

SDL_Texture *Renderer::setRenderTarget(SDL_Texture* tex_)
{
    auto tmp = SDL_GetRenderTarget(m_renderer);
	int i = SDL_SetRenderTarget(m_renderer, tex_);
	if (i != 0)
		std::cout << i << ": " << SDL_GetError() << std::endl;

    return tmp;
}

SDL_Renderer *Renderer::getRenderer()
{
	return m_renderer;
}