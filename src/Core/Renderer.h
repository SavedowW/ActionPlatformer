#ifndef RENDERER_H_
#define RENDERER_H_

#include "Collider.h"
#include "Camera.h"
#include "Shader.h"
#include "Vector2.hpp"

#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Renderer
{
public:
    Renderer(SDL_Window *window_);

    static unsigned int surfaceToTexture(SDL_Surface *sur_);
    static std::vector<unsigned int> surfacesToTexture(const std::vector<SDL_Surface*> &surfaces);
    static unsigned int createTextureRGBA(int width_, int height_);

    void attachTex(unsigned int tex_, const Vector2<unsigned int> &size_);
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
    void drawCircleOutline(const Vector2<int> &center_, float radius_, const SDL_Color& col_);
    void drawCircleOutline(const Vector2<int> &center_, float radius_, const SDL_Color& col_, const Camera &cam_);

    void drawCollider(const Collider &cld_, const SDL_Color &fillCol_, const Camera &cam_);
    void drawCollider(const SlopeCollider &cld_, const SDL_Color &fillCol_, const Camera &cam_);
    void drawCollider(const Collider &cld_, const SDL_Color &fillCol_, const SDL_Color &borderCol_, const Camera &cam_);

    // TODO: templated integral param, to hpp
    void renderTextureOutlined(unsigned int tex_, const Vector2<int> &pos_, const Vector2<int> &size_, SDL_FlipMode flip_);
    void renderTextureOutlined(unsigned int tex_, const Vector2<int> &pos_, const Vector2<int> &size_, SDL_FlipMode flip_, const Camera &cam_);

    void renderTexture(unsigned int tex_, const Vector2<int> &pos_, const Vector2<int> &size_, SDL_FlipMode flip_, float alpha_);
    void renderTexture(unsigned int tex_, const Vector2<int> &pos_, const Vector2<int> &size_, SDL_FlipMode flip_, float alpha_, const Camera &cam_);

    void renderTexture(unsigned int tex_, const Vector2<int> &pos_, const Vector2<int> &size_, SDL_FlipMode flip_, float degrees_, const Vector2<int> &pivot_);
    void renderTexture(unsigned int tex_, const Vector2<int> &pos_, const Vector2<int> &size_, SDL_FlipMode flip_, float degrees_, const Vector2<int> &pivot_, const Camera &cam_);

    void renderTextureFlash(unsigned int tex_, const Vector2<int> &pos_, const Vector2<int> &size_, SDL_FlipMode flip_, uint8_t alpha_);
    void renderTextureFlash(unsigned int tex_, const Vector2<int> &pos_, const Vector2<int> &size_, SDL_FlipMode flip_, uint8_t alpha_, const Camera &cam_);

    void renderTile(unsigned int tex_, const Vector2<int> &pos_, const Vector2<int> &size_, SDL_FlipMode flip_, const Vector2<int> &tilesetPixelsPos_);

private:
    SDL_Window *m_window = nullptr;
    SDL_GLContext m_context = nullptr;

    Shader m_rectShader;
    Shader m_screenShader;
    Shader m_spriteShader;
    Shader m_spriteShaderRotate;
    Shader m_spriteOutlinedShader;
    Shader m_spriteShaderFlash;
    Shader m_tileShader;
    Shader m_circleShader;

    unsigned int m_rectVAO = 0;
    unsigned int m_screenVAO = 0;
    unsigned int m_spriteVAO = 0;

    unsigned int m_renderTarget = 0;
    unsigned int m_renderTargetTexture = 0;

    unsigned int m_renderHudTarget = 0;
    unsigned int m_renderHudTargetTexture = 0;

    unsigned int m_intermTexture = 0;
};

#endif
