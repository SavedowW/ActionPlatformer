#ifndef BACKGROUND_H_
#define BACKGROUND_H_

#include <vector>
#include "TextureManager.h"
#include "Renderer.h"
#include "Camera.h"

struct BackgroundSegment
{
    Texture_t texture;

    BackgroundSegment(Texture_t texture_) {
        texture = texture_;
    }

    const Vector2<float> getMinPosition() const
    {
        return {0.0f, 0.0f};
    }

    const Vector2<float> getMaxPosition(const Vector2<float> &camSize, const float camScale) const
    {
        auto realTexSize = Vector2{float(texture->m_w), float(texture->m_h)} / camScale;
        return (camSize - realTexSize);
    }
};

class Background
{
public:
    Background();

    void draw(Renderer &renderer_, Camera &camera_);
    virtual ~Background() = default;

protected:
    std::vector<BackgroundSegment> m_backgroundSegments;

};

#endif