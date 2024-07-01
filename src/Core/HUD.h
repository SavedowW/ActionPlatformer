#ifndef HUD_H_
#define HUD_H_
#include <vector>
#include <memory>
#include "Renderer.h"

class Widget
{
public:
    Widget() = default;
    virtual void update() = 0;
    virtual void draw(Renderer &renderer_, Camera &camera_) = 0;
    virtual ~Widget() = default;
};

class HUD
{
public:
    HUD();
    void addWidget(std::unique_ptr<Widget> widget_);
    void clear();
    void update();
    void draw(Renderer &renderer_, Camera &camera_);


private:
    std::vector<std::unique_ptr<Widget>> m_widgets;
};

#endif