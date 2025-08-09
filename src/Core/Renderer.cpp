#include "Renderer.h"
#include "FilesystemUtils.h"
#include "GameData.h"
#include "Configuration.h"
#include "JsonUtils.hpp"
#include <SDL_Image.h>
#include <SDL.h>
#include <chrono>

Renderer::Renderer(SDL_Window *window_) :
    m_window(window_)
{
    m_context = SDL_GL_CreateContext( window_ );
    if (!m_context)
        std::cout << "OpenGL context could not be created! SDL Error: " << SDL_GetError() << std::endl;

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Shaders
    m_rectShader.load(Filesystem::getRootDirectory() + "/src/core/Shader/Rect.vert", Filesystem::getRootDirectory() + "/src/core/Shader/Rect.frag");
    m_screenShader.load(Filesystem::getRootDirectory() + "/src/core/Shader/Screen.vert", Filesystem::getRootDirectory() + "/src/core/Shader/Screen.frag");
    m_spriteShader.load(Filesystem::getRootDirectory() + "/src/core/Shader/Sprite.vert", Filesystem::getRootDirectory() + "/src/core/Shader/Sprite.frag");
    m_spriteShaderFlash.load(Filesystem::getRootDirectory() + "/src/core/Shader/Sprite.vert", Filesystem::getRootDirectory() + "/src/core/Shader/SpriteFlash.frag");
    m_spriteShaderRotate.load(Filesystem::getRootDirectory() + "/src/core/Shader/SpriteRotate.vert", Filesystem::getRootDirectory() + "/src/core/Shader/Sprite.frag");
    m_spriteOutlinedShader.load(Filesystem::getRootDirectory() + "/src/core/Shader/Sprite.vert", Filesystem::getRootDirectory() + "/src/core/Shader/SpriteOutlined.frag");
    m_tileShader.load(Filesystem::getRootDirectory() + "/src/core/Shader/Tilemap.vert", Filesystem::getRootDirectory() + "/src/core/Shader/Sprite.frag");
    m_circleShader.load(Filesystem::getRootDirectory() + "/src/core/Shader/Rect.vert", Filesystem::getRootDirectory() + "/src/core/Shader/Circle.frag");

    unsigned int rectVBO;
    uint32_t rectVertices[] = { // TL, TR, BR, BL
        0,
        1,
        2,
        3,
        0,
        2
    };

    float texCoords[] = {
        0.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        2.0f, 1.0f, 1.0f,
        3.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 0.0f,
        2.0f, 1.0f, 1.0f
    };

    // To render framebuffer on screen
    float screenVertices[] = {
        // positions          // texture Coords
        -1.0f, -1.0f,  0.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  0.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  0.0f,  1.0f, 1.0f,

        -1.0f, -1.0f,  0.0f,  0.0f, 0.0f,
         1.0f,  1.0f,  0.0f,  1.0f, 1.0f,					
        -1.0f,  1.0f,  0.0f,  0.0f, 1.0f
    };

    // Rectangle indices
    glGenVertexArrays(1, &m_rectVAO);  
    glGenBuffers(1, &rectVBO);
    glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rectVertices), rectVertices, GL_STATIC_DRAW);
    glBindVertexArray(m_rectVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, 1 * sizeof(uint32_t), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    // screen
    unsigned int quadVBO;
    glGenVertexArrays(1, &m_screenVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(m_screenVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(screenVertices), &screenVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);


    // sprite
    unsigned int spriteVBO;
    glGenVertexArrays(1, &m_spriteVAO);
    glGenBuffers(1, &spriteVBO);
    glBindVertexArray(m_spriteVAO);
    glBindBuffer(GL_ARRAY_BUFFER, spriteVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), &texCoords, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0);


    // Projection matrix
    glm::mat4 projection = glm::ortho(0.0f, 640.0f, 
        360.0f, 0.0f, -1.0f, 1.0f);
    m_rectShader.use();
    m_rectShader.setMatrix4("projection", projection);

    m_screenShader.use();
    m_screenShader.setInteger("screenTexture", 0);

    m_spriteOutlinedShader.use();
    m_spriteOutlinedShader.setInteger("image", 0);
    m_spriteOutlinedShader.setInteger("backi", 1);
    m_spriteOutlinedShader.setMatrix4("projection", projection);

    m_spriteShader.use();
    m_spriteShader.setInteger("image", 0);
    m_spriteShader.setMatrix4("projection", projection);
    m_spriteShader.setFloat("alphaMod", 1.0f);

    m_spriteShaderFlash.use();
    m_spriteShaderFlash.setInteger("image", 0);
    m_spriteShaderFlash.setMatrix4("projection", projection);

    m_spriteShaderRotate.use();
    m_spriteShaderRotate.setInteger("image", 0);
    m_spriteShaderRotate.setMatrix4("projection", projection);
    m_spriteShaderRotate.setFloat("alphaMod", 1.0f);

    m_tileShader.use();
    m_tileShader.setInteger("image", 0);
    m_tileShader.setMatrix4("projection", projection);
    m_tileShader.setFloat("alphaMod", 1.0f);

    m_circleShader.use();
    m_circleShader.setMatrix4("projection", projection);


    // Framebuffers
    glGenFramebuffers(1, &m_renderTarget);
    glBindFramebuffer(GL_FRAMEBUFFER, m_renderTarget);

    // Generating texture
    glGenTextures(1, &m_renderTargetTexture);
    glBindTexture(GL_TEXTURE_2D, m_renderTargetTexture);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, gamedata::global::maxCameraSize.x, gamedata::global::maxCameraSize.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Binding texture
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_renderTargetTexture, 0); 

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	    std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // Hud framebuffer
    glGenFramebuffers(1, &m_renderHudTarget);
    glBindFramebuffer(GL_FRAMEBUFFER, m_renderHudTarget);

    // Generating texture
    glGenTextures(1, &m_renderHudTargetTexture);
    glBindTexture(GL_TEXTURE_2D, m_renderHudTargetTexture);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, gamedata::global::hudLayerResolution.x, gamedata::global::hudLayerResolution.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Binding texture
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_renderHudTarget, 0); 

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	    std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // Generating intermediate texture
    glGenTextures(1, &m_intermTexture);
    glBindTexture(GL_TEXTURE_2D, m_intermTexture);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, gamedata::global::maxCameraSize.x, gamedata::global::maxCameraSize.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
}

unsigned int Renderer::surfaceToTexture(SDL_Surface *sur_)
{
    if (!sur_)
    {
        std::cout << "Trying to create texture from non-existing surface" << std::endl;
        return -1;
    }

    unsigned int res;

    glGenTextures(1, &res);
    glBindTexture(GL_TEXTURE_2D, res);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sur_->w, sur_->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, sur_->pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);

    dumpErrors();

    return res;
}

unsigned int *Renderer::surfacesToTexture(const std::vector<SDL_Surface *> &surfaces_)
{
    for (const auto &el : surfaces_)
    {
        if (!el)
        {
            std::cout << "Trying to create texture from non-existing surface in array" << std::endl;
            return nullptr;
        }
    }

    unsigned int *ids = new unsigned int[surfaces_.size()];

    glGenTextures(surfaces_.size(), ids);

    for (size_t i = 0; i < surfaces_.size(); ++i)
    {
        glBindTexture(GL_TEXTURE_2D, ids[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surfaces_[i]->w, surfaces_[i]->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surfaces_[i]->pixels);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glBindTexture(GL_TEXTURE_2D, 0);

        dumpErrors();
    }

    return ids;
}

unsigned int Renderer::createTextureRGBA(int width_, int height_)
{
    unsigned int texid;

    // Generating intermediate texture
    glGenTextures(1, &texid);
    glBindTexture(GL_TEXTURE_2D, texid);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texid;
}

void Renderer::attachTex(unsigned int tex_, const Vector2<unsigned int> &size_)
{
    glViewport(0, 0, size_.x, size_.y);
    glBindFramebuffer(GL_FRAMEBUFFER, m_renderTarget);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_, 0); 

    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(size_.x), 
        static_cast<float>(size_.y), 0.0f, -1.0f, 1.0f);
    m_spriteShader.use();
    m_spriteShader.setMatrix4("projection", projection);
}

void Renderer::attachTex()
{
    glViewport(0, 0, gamedata::global::maxCameraSize.x, gamedata::global::maxCameraSize.y);
    glBindFramebuffer(GL_FRAMEBUFFER, m_renderTarget);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_renderTargetTexture, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	    std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;


    glm::mat4 projection = glm::ortho(0.0f, 640.0f, 
        360.0f, 0.0f, -1.0f, 1.0f);
    m_spriteShader.use();
    m_spriteShader.setMatrix4("projection", projection);
}

void Renderer::prepareRenderer(const SDL_Color &col_)
{
    glViewport(0, 0, 640, 360);
    glBindFramebuffer(GL_FRAMEBUFFER, m_renderTarget);
    fillRenderer(col_);
}

void Renderer::switchToHUD(const SDL_Color &col_)
{
    glViewport(0, 0, gamedata::global::hudLayerResolution.x, gamedata::global::hudLayerResolution.y);
    glBindFramebuffer(GL_FRAMEBUFFER, m_renderHudTarget);
    fillRenderer(col_);
}

void Renderer::fillRenderer(const SDL_Color &col_)
{
    glClearColor(col_.r / 255.0f, col_.g / 255.0f, col_.b / 255.0f, col_.a / 255.0f);
    glClear( GL_COLOR_BUFFER_BIT );
}

void Renderer::updateScreen(const Camera &cam_)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    const auto resolution = ConfigurationManager::instance().m_settings["video"]["window_resolution"].readOrSet<Vector2<int>>({1920, 1080});
    glViewport(0, 0, resolution.x, resolution.y);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    m_screenShader.use();
    m_screenShader.setFloat("scale", cam_.getScale());
    
    glBindVertexArray(m_screenVAO);
    glBindTexture(GL_TEXTURE_2D, m_renderTargetTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    m_screenShader.setFloat("scale", 1.0f);
    
    glBindTexture(GL_TEXTURE_2D, m_renderHudTargetTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    SDL_GL_SwapWindow( m_window );
}

void Renderer::drawRectangle(const Vector2<int> &pos_, const Vector2<int> &size_, const SDL_Color &col_)
{
    glBindVertexArray(m_rectVAO);
    m_rectShader.use();

    m_rectShader.setVector4f("color", col_.r / 255.0f, col_.g / 255.0f, col_.b / 255.0f, col_.a / 255.0f);
    m_rectShader.setVector2f("vertices[0]", pos_.x + 0.375f, pos_.y + 0.375f);
    m_rectShader.setVector2f("vertices[1]", pos_.x + 0.375f + size_.x - 1, pos_.y + 0.375f);
    m_rectShader.setVector2f("vertices[2]", pos_.x + 0.375f + size_.x - 1, pos_.y + 0.375f + size_.y - 1);
    m_rectShader.setVector2f("vertices[3]", pos_.x + 0.375f, pos_.y + 0.375f + size_.y - 1);
    glDrawArrays(GL_LINE_LOOP, 0, 4);
}

void Renderer::drawRectangle(const Vector2<int> &pos_, const Vector2<int> &size_, const SDL_Color &col_, const Camera &cam_)
{
    Vector2<int> camTL = Vector2<int>(cam_.getPos() - gamedata::global::maxCameraSize / 2.0f);
    drawRectangle(pos_ - camTL, size_, col_);
}

void Renderer::fillRectangle(const Vector2<int> &pos_, const Vector2<int> &size_, const SDL_Color &col_)
{
    glBindVertexArray(m_rectVAO);
    m_rectShader.use();

    m_rectShader.setVector4f("color", col_.r / 255.0f, col_.g / 255.0f, col_.b / 255.0f, col_.a / 255.0f);
    
    m_rectShader.setVector2f("vertices[0]", pos_.x, pos_.y);
    m_rectShader.setVector2f("vertices[1]", pos_.x + size_.x, pos_.y);
    m_rectShader.setVector2f("vertices[2]", pos_.x + size_.x, pos_.y + size_.y);
    m_rectShader.setVector2f("vertices[3]", pos_.x, pos_.y + size_.y);

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Renderer::fillRectangle(const Vector2<int> &pos_, const Vector2<int> &size_, const SDL_Color &col_, const Camera &cam_)
{
    Vector2<int> camTL = Vector2<int>(cam_.getPos() - gamedata::global::maxCameraSize / 2.0f);
    fillRectangle(pos_ - camTL, size_, col_);
}

void Renderer::drawLine(const Vector2<int> &p1_, const Vector2<int> &p2_, const SDL_Color &col_)
{
    glBindVertexArray(m_rectVAO);
    m_rectShader.use();

    m_rectShader.setVector4f("color", col_.r / 255.0f, col_.g / 255.0f, col_.b / 255.0f, col_.a / 255.0f);
    
    m_rectShader.setVector2f("vertices[0]", p1_.x, p1_.y);
    m_rectShader.setVector2f("vertices[1]", p2_.x, p2_.y);

    glDrawArrays(GL_LINES, 0, 2);
}

void Renderer::drawLine(const Vector2<int> &p1_, const Vector2<int> &p2_, const SDL_Color &col_, const Camera &cam_)
{
    Vector2<int> camTL = Vector2<int>(cam_.getPos() - gamedata::global::maxCameraSize / 2.0f);
    drawLine(p1_ - camTL, p2_ - camTL, col_);
}

void Renderer::drawCross(const Vector2<int> &center_, const Vector2<int> &vSize_, const Vector2<int> &hSize_, const SDL_Color &col_, const Camera &cam_)
{
    Vector2<int> camTL = Vector2<int>(cam_.getPos() - gamedata::global::maxCameraSize / 2.0f);
	drawCross(center_ - camTL, vSize_, hSize_, col_);
}

void Renderer::drawCircleOutline(const Vector2<int> &center_, float radius_, const SDL_Color &col_)
{
    glBindVertexArray(m_rectVAO);
    m_circleShader.use();

    m_circleShader.setVector4f("color", col_.r / 255.0f, col_.g / 255.0f, col_.b / 255.0f, col_.a / 255.0f);
    
    m_circleShader.setVector2f("vertices[0]", center_.x - radius_, center_.y - radius_);
    m_circleShader.setVector2f("vertices[1]", center_.x + radius_, center_.y - radius_);
    m_circleShader.setVector2f("vertices[2]", center_.x + radius_, center_.y + radius_);
    m_circleShader.setVector2f("vertices[3]", center_.x - radius_, center_.y + radius_);

    m_circleShader.setFloat("radius", radius_);

    m_circleShader.setVector2f("center", center_.x, center_.y);

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Renderer::drawCircleOutline(const Vector2<int> &center_, float radius_, const SDL_Color &col_, const Camera &cam_)
{
    Vector2<int> camTL = Vector2<int>(cam_.getPos() - gamedata::global::maxCameraSize / 2.0f);
    drawCircleOutline(center_ - camTL, radius_, col_);
}

void Renderer::drawCross(const Vector2<int> &center_, const Vector2<int> &vSize_, const Vector2<int> &hSize_, const SDL_Color &col_)
{
    drawRectangle({center_.x - vSize_.x / 2, center_.y - vSize_.y / 2}, {vSize_.x, vSize_.y}, col_);
    drawRectangle({center_.x - hSize_.x / 2, center_.y - hSize_.y / 2}, {hSize_.x, hSize_.y }, col_);
}

void Renderer::drawCollider(const Collider &cld_, const SDL_Color &fillCol_, const Camera &cam_)
{
    fillRectangle(cld_.m_topLeft, cld_.m_size, fillCol_, cam_);
}

void Renderer::drawCollider(const SlopeCollider &cld_, const SDL_Color &fillCol_, const Camera &cam_)
{
    Vector2<int> camTL = Vector2<int>(cam_.getPos() - gamedata::global::maxCameraSize / 2.0f);

    glBindVertexArray(m_rectVAO);
    m_rectShader.use();

    m_rectShader.setVector4f("color", fillCol_.r / 255.0f, fillCol_.g / 255.0f, fillCol_.b / 255.0f, fillCol_.a / 255.0f);
    
    m_rectShader.setVector2f("vertices[0]", cld_.m_points[0].x - camTL.x, cld_.m_points[0].y - camTL.y);
    m_rectShader.setVector2f("vertices[1]", cld_.m_points[1].x - camTL.x + 1, cld_.m_points[1].y - camTL.y);
    m_rectShader.setVector2f("vertices[2]", cld_.m_points[2].x - camTL.x + 1, cld_.m_points[2].y - camTL.y + 1);
    m_rectShader.setVector2f("vertices[3]", cld_.m_points[3].x - camTL.x, cld_.m_points[3].y - camTL.y + 1);

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Renderer::drawCollider(const Collider &cld_, const SDL_Color &fillCol_, const SDL_Color &borderCol_, const Camera &cam_)
{
    fillRectangle(cld_.m_topLeft, cld_.m_size, fillCol_, cam_);
    drawRectangle(cld_.m_topLeft, cld_.m_size, borderCol_, cam_);
}

void Renderer::renderTextureOutlined(const unsigned int tex_, const Vector2<int> &pos_, const Vector2<int> &size_, SDL_RendererFlip flip_)
{
    glCopyImageSubData(
        m_renderTargetTexture, GL_TEXTURE_2D, 0, 0, 0, 0,
        m_intermTexture, GL_TEXTURE_2D, 0, 0, 0, 0,
        640, 360, 1
    );
    
    glBindVertexArray(m_spriteVAO);
    m_spriteOutlinedShader.use();

    int top, bot, lft, rgt;
    if (flip_ & SDL_FLIP_VERTICAL)
    {
        top = pos_.y + size_.y;
        bot = pos_.y;
    }
    else
    {
        top = pos_.y;
        bot = pos_.y + size_.y;
    }
    if (flip_ & SDL_FLIP_HORIZONTAL)
    {
        lft = pos_.x + size_.x;
        rgt = pos_.x;
    }
    else
    {
        lft = pos_.x;
        rgt = pos_.x + size_.x;
    }


    Vector2<int> vertices[] =
    {
        {lft, top},
        {rgt, top},
        {rgt, bot},
        {lft, bot}
    };

    m_spriteOutlinedShader.setVector2f("vertices[0]", vertices[0].x, vertices[0].y);
    m_spriteOutlinedShader.setVector2f("vertices[1]", vertices[1].x, vertices[1].y);
    m_spriteOutlinedShader.setVector2f("vertices[2]", vertices[2].x, vertices[2].y);
    m_spriteOutlinedShader.setVector2f("vertices[3]", vertices[3].x, vertices[3].y);

    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, tex_);
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, m_intermTexture);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glActiveTexture(GL_TEXTURE0 + 0);
}

void Renderer::renderTextureOutlined(const unsigned int tex_, const Vector2<int> &pos_, const Vector2<int> &size_, SDL_RendererFlip flip_, const Camera &cam_)
{
    Vector2<int> camTL = Vector2<int>(cam_.getPos() - gamedata::global::maxCameraSize / 2.0f);
    renderTextureOutlined(tex_, pos_ - camTL, size_, flip_);
}

void Renderer::renderTexture(const unsigned int tex_, const Vector2<int> &pos_, const Vector2<int> &size_, SDL_RendererFlip flip_, float alpha_)
{
    glBindVertexArray(m_spriteVAO);
    m_spriteShader.use();

    int top, bot, lft, rgt;
    if (flip_ & SDL_FLIP_VERTICAL)
    {
        top = pos_.y + size_.y;
        bot = pos_.y;
    }
    else
    {
        top = pos_.y;
        bot = pos_.y + size_.y;
    }
    if (flip_ & SDL_FLIP_HORIZONTAL)
    {
        lft = pos_.x + size_.x;
        rgt = pos_.x;
    }
    else
    {
        lft = pos_.x;
        rgt = pos_.x + size_.x;
    }


    Vector2<int> vertices[] =
    {
        {lft, top},
        {rgt, top},
        {rgt, bot},
        {lft, bot}
    };

    m_spriteShader.setVector2f("vertices[0]", vertices[0].x, vertices[0].y);
    m_spriteShader.setVector2f("vertices[1]", vertices[1].x, vertices[1].y);
    m_spriteShader.setVector2f("vertices[2]", vertices[2].x, vertices[2].y);
    m_spriteShader.setVector2f("vertices[3]", vertices[3].x, vertices[3].y);
    m_spriteShader.setFloat("alphaMod", alpha_);

    glBindTexture(GL_TEXTURE_2D, tex_);

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Renderer::renderTexture(const unsigned int tex_, const Vector2<int> &pos_, const Vector2<int> &size_, SDL_RendererFlip flip_, float alpha_, const Camera &cam_)
{
    Vector2<int> camTL = Vector2<int>(cam_.getPos() - gamedata::global::maxCameraSize / 2.0f);
    renderTexture(tex_, pos_ - camTL, size_, flip_, alpha_);
}

void Renderer::renderTexture(const unsigned int tex_, const Vector2<int> &pos_, const Vector2<int> &size_, SDL_RendererFlip flip_, float degrees_, const Vector2<int> pivot_)
{
    glBindVertexArray(m_spriteVAO);
    m_spriteShaderRotate.use();

    auto realPivot = pivot_ + pos_;

    int top, bot, lft, rgt;
    if (flip_ & SDL_FLIP_VERTICAL)
    {
        top = pos_.y + size_.y;
        bot = pos_.y;
    }
    else
    {
        top = pos_.y;
        bot = pos_.y + size_.y;
    }
    if (flip_ & SDL_FLIP_HORIZONTAL)
    {
        lft = pos_.x + size_.x;
        rgt = pos_.x;
    }
    else
    {
        lft = pos_.x;
        rgt = pos_.x + size_.x;
    }


    Vector2<int> vertices[] =
    {
        {lft, top},
        {rgt, top},
        {rgt, bot},
        {lft, bot}
    };

    m_spriteShaderRotate.setVector2f("vertices[0]", vertices[0].x, vertices[0].y);
    m_spriteShaderRotate.setVector2f("vertices[1]", vertices[1].x, vertices[1].y);
    m_spriteShaderRotate.setVector2f("vertices[2]", vertices[2].x, vertices[2].y);
    m_spriteShaderRotate.setVector2f("vertices[3]", vertices[3].x, vertices[3].y);
    m_spriteShaderRotate.setFloat("angle", utils::degreesToRadians(degrees_));
    m_spriteShaderRotate.setVector2f("pivotPoint", realPivot.x, realPivot.y);

    glBindTexture(GL_TEXTURE_2D, tex_);

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Renderer::renderTexture(const unsigned int tex_, const Vector2<int> &pos_, const Vector2<int> &size_, SDL_RendererFlip flip_, float degrees_, const Vector2<int> pivot_, const Camera &cam_)
{
    Vector2<int> camTL = Vector2<int>(cam_.getPos() - gamedata::global::maxCameraSize / 2.0f);
    renderTexture(tex_, pos_ - camTL, size_, flip_, degrees_, pivot_);
}

void Renderer::renderTextureFlash(const unsigned int tex_, const Vector2<int> &pos_, const Vector2<int> &size_, SDL_RendererFlip flip_, uint8_t alpha_)
{
    glBindVertexArray(m_spriteVAO);
    m_spriteShaderFlash.use();

    int top, bot, lft, rgt;
    if (flip_ & SDL_FLIP_VERTICAL)
    {
        top = pos_.y + size_.y;
        bot = pos_.y;
    }
    else
    {
        top = pos_.y;
        bot = pos_.y + size_.y;
    }
    if (flip_ & SDL_FLIP_HORIZONTAL)
    {
        lft = pos_.x + size_.x;
        rgt = pos_.x;
    }
    else
    {
        lft = pos_.x;
        rgt = pos_.x + size_.x;
    }


    Vector2<int> vertices[] =
    {
        {lft, top},
        {rgt, top},
        {rgt, bot},
        {lft, bot}
    };

    m_spriteShaderFlash.setVector2f("vertices[0]", vertices[0].x, vertices[0].y);
    m_spriteShaderFlash.setVector2f("vertices[1]", vertices[1].x, vertices[1].y);
    m_spriteShaderFlash.setVector2f("vertices[2]", vertices[2].x, vertices[2].y);
    m_spriteShaderFlash.setVector2f("vertices[3]", vertices[3].x, vertices[3].y);
    m_spriteShaderFlash.setFloat("alphaMod", alpha_ / 255.0f);

    glBindTexture(GL_TEXTURE_2D, tex_);

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Renderer::renderTextureFlash(const unsigned int tex_, const Vector2<int> &pos_, const Vector2<int> &size_, SDL_RendererFlip flip_, uint8_t alpha_, const Camera &cam_)
{
    Vector2<int> camTL = Vector2<int>(cam_.getPos() - gamedata::global::maxCameraSize / 2.0f);
    renderTextureFlash(tex_, pos_ - camTL, size_, flip_, alpha_);
}

void Renderer::renderTile(const unsigned int tex_, const Vector2<int> &pos_, const Vector2<int> &size_, SDL_RendererFlip flip_, const Vector2<int> &tilesetPixelsPos_)
{
    glBindVertexArray(m_spriteVAO);
    m_tileShader.use();

    int top, bot, lft, rgt;
    if (flip_ & SDL_FLIP_VERTICAL)
    {
        top = pos_.y + size_.y;
        bot = pos_.y;
    }
    else
    {
        top = pos_.y;
        bot = pos_.y + size_.y;
    }
    if (flip_ & SDL_FLIP_HORIZONTAL)
    {
        lft = pos_.x + size_.x;
        rgt = pos_.x;
    }
    else
    {
        lft = pos_.x;
        rgt = pos_.x + size_.x;
    }


    Vector2<int> vertices[] =
    {
        {lft, top},
        {rgt, top},
        {rgt, bot},
        {lft, bot}
    };

    m_tileShader.setVector2f("vertices[0]", vertices[0].x, vertices[0].y);
    m_tileShader.setVector2f("vertices[1]", vertices[1].x, vertices[1].y);
    m_tileShader.setVector2f("vertices[2]", vertices[2].x, vertices[2].y);
    m_tileShader.setVector2f("vertices[3]", vertices[3].x, vertices[3].y);
    m_tileShader.setVector2f("tilePos", tilesetPixelsPos_.x, tilesetPixelsPos_.y);

    glBindTexture(GL_TEXTURE_2D, tex_);

    glDrawArrays(GL_TRIANGLES, 0, 6);
}
