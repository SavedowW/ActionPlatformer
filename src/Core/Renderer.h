#ifndef RENDERER_H_
#define RENDERER_H_

#include "Window.h"
#include "Camera.h"
#include "Texture.h"
#include "Collider.h"

class Renderer
{
public:
    Renderer(Window &window_);
    ~Renderer();

    Renderer& operator=(const Renderer &rhs) = delete;
    Renderer(const Renderer &rhs) = delete;

    Renderer& operator=(Renderer &&rhs);
    Renderer(Renderer &&rhs);

    //Texture creation
    SDL_Texture* createTexture(int w_, int h_);
    SDL_Texture* createTexture(const Vector2<int> &size_);
    SDL_Texture* loadTexture(const char* file_);
    SDL_Texture* createTextureFromSurface(SDL_Surface* sur_);

    //Texture render
    void renderTexture(SDL_Texture* tex_, float x_, float y_, float w_, float h_);
    //void renderTexture(SDL_Texture* tex_, float x_, float y_, const Camera &cam_, SDL_RendererFlip flip_ = SDL_FLIP_NONE);
    //void renderTexture(SDL_Texture* tex_, float x_, float y_);
    void renderTexture(SDL_Texture* tex_, float x_, float y_, float w_, float h_, float angle_, SDL_FPoint* center_, SDL_RendererFlip flip_);
    void renderTexture(SDL_Texture* tex_, float x_, float y_, float w_, float h_, const Camera &cam_, float angle_, const Vector2<float> &center_, SDL_RendererFlip flip_);
    void renderTexture(SDL_Texture* tex_, float x_, float y_, float w_, float h_, const Camera &cam_, float angle_, SDL_RendererFlip flip_);
    void renderTexture(SDL_Texture* tex_, const SDL_Rect *src, const SDL_Rect *dst, float angle_, SDL_Point* center_, SDL_RendererFlip flip_);
    void renderTexture(SDL_Texture* tex_, float x_, float y_, float w_, float h_, const SDL_Rect &src, const Camera &cam_, SDL_RendererFlip flip_);
    //void renderTexture(SDL_Texture* tex_, float x_, float y_, float angle_, SDL_FPoint* center_, SDL_RendererFlip flip_);

    //Primitives
    void drawLine(const Vector2<float> &p1_, const Vector2<float> &p2_, const SDL_Color& col_, const Camera &cam_);
    void drawRectangle(const Vector2<float> &pos_, const Vector2<float> &size_, const SDL_Color& col_, const Camera &cam_);
    void drawRectangle(const Vector2<float> &pos_, const Vector2<float> &size_, const SDL_Color& col_);
    void fillRectangle(const Vector2<float> &pos_, const Vector2<float> &size_, const SDL_Color& col_, const Camera &cam_);
    void fillRectangle(const Vector2<float> &pos_, const Vector2<float> &size_, const SDL_Color& col_);
    void drawCross(const Vector2<float> &center_, const Vector2<float> &vSize_, const Vector2<float> &hSize_, const SDL_Color& col_, const Camera &cam_);
    void drawCross(const Vector2<float> &center_, const Vector2<float> &vSize_, const Vector2<float> &hSize_, const SDL_Color& col_);
    void drawGeometry(SDL_Texture *texture, const SDL_Vertex *vertices, int num_vertices, const int *indices, int num_indices);

    void drawCollider(const Collider &cld_, const SDL_Color &fillCol_, const Camera &cam_);
    void drawCollider(const SlopeCollider &cld_, const SDL_Color &fillCol_, const Camera &cam_);
    void drawCollider(const Collider &cld_, const SDL_Color &fillCol_, const SDL_Color &borderCol_, const Camera &cam_);
    void drawCollider(const SlopeCollider &cld_, const SDL_Color &fillCol_, const SDL_Color &borderCol_, const Camera &cam_);

    //Global render
    void prepareRenderer(const SDL_Color &col_);
    void switchToHUD(const SDL_Color &col_);
    void fillRenderer(const SDL_Color &col_);
    void updateScreen(const Camera &cam_);

    //Render target settings
    SDL_Texture *setRenderTarget(SDL_Texture* tex_ = NULL);

	SDL_Renderer *getRenderer();

//private:
    SDL_Renderer* m_renderer = nullptr;
    Texture m_backbuffGameplay;
	Texture m_backbuffHUD;
};

#endif
