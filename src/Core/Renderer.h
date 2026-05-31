#pragma once
#include "Window.h"
#include "Collider.h"
#include "Camera.h"
#include "Shader.h"
#include "Texture.h"
#include "Vector2.hpp"
#include "Color.hpp"
#include "Framebuffer.h"
#include <SDL3/SDL.h>

/**
 *  TODO:
 *   - avoid constant state switching
 *   - exclude SDL types from interface
 */
class Renderer
{
public:
    Renderer(const Window &window_);

    static std::vector<unsigned int> surfacesToTexture(const std::vector<SDL_Surface*> &surfaces);

    // Switch to provided texture as a target
    void setTarget(const Texture &texture_);
    void resetTarget();

    void switchToWorld(const Color &col_);
    void switchToHUD(const Color &col_);
    void switchToDBG(const Color &col_);
    void fillRenderer(const Color &col_);
    void updateScreen(const Camera &cam_);

    // Line, cross
    void drawRectangle(const Vector2<int> &pos_, const Vector2<int> &size_, const Color& col_);
    void drawRectangle(const Vector2<int> &pos_, const Vector2<int> &size_, const Color& col_, const Camera &cam_);
    void fillRectangle(const Vector2<int> &pos_, const Vector2<int> &size_, const Color& col_);
    void fillRectangle(const Vector2<int> &pos_, const Vector2<int> &size_, const Color& col_, const Camera &cam_);
    void drawLine(const Vector2<int> &p1_, const Vector2<int> &p2_, const Color& col_);
    void drawLine(const Vector2<int> &p1_, const Vector2<int> &p2_, const Color& col_, const Camera &cam_);
    void drawCross(const Vector2<int> &center_, const Vector2<int> &vSize_, const Vector2<int> &hSize_, const Color& col_);
    void drawCross(const Vector2<int> &center_, const Vector2<int> &vSize_, const Vector2<int> &hSize_, const Color& col_, const Camera &cam_);
    void drawCircleOutline(const Vector2<int> &center_, float radius_, const Color& col_);
    void drawCircleOutline(const Vector2<int> &center_, float radius_, const Color& col_, const Camera &cam_);

    void drawCollider(const Collider &cld_, const Color &fillCol_, const Camera &cam_);
    void drawCollider(const SlopeCollider &cld_, const Color &fillCol_, const Camera &cam_);
    void drawCollider(const Collider &cld_, const Color &fillCol_, const Color &borderCol_, const Camera &cam_);

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
    void selectTarget(const Framebuffer &fb_, const Vector2<int> &size_);

    const Window &m_window;
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

    // Framebuffer for world entities,
    Framebuffer m_worldFB;

    // Framebuffer used only for in-game HUD, uses specified resolution
    Framebuffer m_hudFB;

    // Framebuffer used only for debugging HUD, uses window resolution
    Framebuffer m_dbgFB;

    // Framebuffer used only for custom targets 
    Framebuffer m_customFB;

    enum class Stage : uint8_t
    {
        WORLD,
        HUD,
        DBG
    } m_stage = Stage::WORLD;


    // Texture only for world objects, uses max camera size
    Texture m_renderWorldTargetTexture;

    // Texture for HUD framebuffer
    Texture m_renderHudTargetTexture;

    // Texture for debugging HUD rendering
    Texture m_renderDbgTargetTexture;

    /**
     *  Texture used for outline effect
     *  since opengl does not define behavior when reading texture
     *  that is currently used for rendering
     */
    Texture m_intermTexture;
};
