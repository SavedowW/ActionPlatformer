#include "Background.h"

Background::Background()
{
}

void Background::draw(Renderer &renderer_, Camera &camera_)
{
    /*std::cout << "Cam size: " << camera_.getSize() << std::endl;
    std::cout << "Area size: " << camera_.getAreaSize() << std::endl;
    std::cout << "Cam pos: " << camera_.getPos() << std::endl;
    std::cout << "Cam top left: " << camera_.getTopLeft() << std::endl;
    std::cout << "Cam pos normalized: " << camera_.getPositionNormalizedValues() << std::endl;
    std::cout << "Cam scale: " << camera_.getScale() << std::endl;*/
    for (auto &el : m_backgroundSegments)
    {
        auto minPos = el.getMinPosition();
        auto maxPos = el.getMaxPosition(camera_.getBaseSize(), camera_.getScale());

        //std::cout << "minPos: " << minPos << std::endl;
        //std::cout << "maxPos: " << maxPos << std::endl;

        auto range = maxPos - minPos;
        auto camPosNormalized = camera_.getPositionNormalizedValues();

        //std::cout << "Pos offset: " << range.mulComponents(camPosNormalized) << std::endl;

        auto pos = minPos + range.mulComponents(camPosNormalized);
        //std::cout << "Rendering texture at " << pos << std::endl;
        renderer_.renderTexture(el.texture->getSprite(), pos.x, pos.y, el.texture->m_w / camera_.getScale(), el.texture->m_h / camera_.getScale());
    }
}