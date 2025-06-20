#ifndef RENDERER_H_
#define RENDERER_H_

#include "Collider.h"
#include "Camera.h"
#include "Shader.h"
#include "Vector2.h"
#include "Texture.h"

#include <glad/glad.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

class Renderer
{
public:
    Renderer(SDL_Window *window_);

    static unsigned int surfaceToTexture(SDL_Surface *sur_);
    static unsigned int *surfacesToTexture(const std::vector<SDL_Surface*> &surfaces);
    static unsigned int createTextureRGBA(int width_, int height_);

    void attachTex(unsigned int tex_, const Vector2<float> &size_);
    void attachTex();

    void prepareRenderer(const SDL_Color &col_);
    void switchToHUD(const SDL_Color &col_);
    void fillRenderer(const SDL_Color &col_);
    void updateScreen(const Camera &cam_);

    // Line, cross
    void drawRectangle(const Vector2<int> &pos_, const Vector2<int> &size_, const SDL_Color& col_);
    void drawRectangle(const Vector2<int> &pos_, const Vector2<int> &size_, const SDL_Color& col_, const Camera &cam_);
    void fillRectangle(const Vector2<int> &pos_, const Vector2<int> &size_, const SDL_Color& col_);
    void fillRectangle(const Vector2<int> &pos_, const Vector2<int> &size_, const SDL_Color& col_, const Camera &cam_);
    void drawLine(const Vector2<int> &p1_, const Vector2<int> &p2_, const SDL_Color& col_);
    void drawLine(const Vector2<int> &p1_, const Vector2<int> &p2_, const SDL_Color& col_, const Camera &cam_);
    void drawCross(const Vector2<int> &center_, const Vector2<int> &vSize_, const Vector2<int> &hSize_, const SDL_Color& col_);
    void drawCross(const Vector2<int> &center_, const Vector2<int> &vSize_, const Vector2<int> &hSize_, const SDL_Color& col_, const Camera &cam_);

    void drawCollider(const Collider &cld_, const SDL_Color &fillCol_, const Camera &cam_);
    void drawCollider(const SlopeCollider &cld_, const SDL_Color &fillCol_, const Camera &cam_);
    void drawCollider(const Collider &cld_, const SDL_Color &fillCol_, const SDL_Color &borderCol_, const Camera &cam_);

    // Add horizontal flip
    // Version with flip + angle
    // Tilemap version
    void renderTextureOutlined(const unsigned int tex_, const Vector2<int> &pos_, const Vector2<int> &size_, SDL_RendererFlip flip_);
    void renderTextureOutlined(const unsigned int tex_, const Vector2<int> &pos_, const Vector2<int> &size_, SDL_RendererFlip flip_, const Camera &cam_);

    void renderTexture(const unsigned int tex_, const Vector2<int> &pos_, const Vector2<int> &size_, SDL_RendererFlip flip_, float alpha_);
    void renderTexture(const unsigned int tex_, const Vector2<int> &pos_, const Vector2<int> &size_, SDL_RendererFlip flip_, float alpha_, const Camera &cam_);

    void renderTexture(const unsigned int tex_, const Vector2<int> &pos_, const Vector2<int> &size_, SDL_RendererFlip flip_, float degrees_, const Vector2<int> pivot_);
    void renderTexture(const unsigned int tex_, const Vector2<int> &pos_, const Vector2<int> &size_, SDL_RendererFlip flip_, float degrees_, const Vector2<int> pivot_, const Camera &cam_);

    void renderTextureFlash(const unsigned int tex_, const Vector2<int> &pos_, const Vector2<int> &size_, SDL_RendererFlip flip_, uint8_t alpha_);
    void renderTextureFlash(const unsigned int tex_, const Vector2<int> &pos_, const Vector2<int> &size_, SDL_RendererFlip flip_, uint8_t alpha_, const Camera &cam_);

    void renderTile(const unsigned int tex_, const Vector2<int> &pos_, const Vector2<int> &size_, SDL_RendererFlip flip_, const Vector2<int> &tilesetPixelsPos_);

private:
    SDL_GLContext m_context;
    SDL_Window *m_window;

    Shader m_rectShader;
    Shader m_screenShader;
    Shader m_spriteShader;
    Shader m_spriteShaderRotate;
    Shader m_spriteOutlinedShader;
    Shader m_spriteShaderFlash;
    Shader m_tileShader;

    unsigned int m_rectVAO;
    unsigned int m_screenVAO;
    unsigned int m_spriteVAO;

    unsigned int m_renderTarget;
    unsigned int m_renderTargetTexture;

    unsigned int m_renderHudTarget;
    unsigned int m_renderHudTargetTexture;

    unsigned int m_intermTexture;
};

#endif
