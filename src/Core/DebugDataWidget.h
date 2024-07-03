#ifndef DEBUG_DATA_WIDGET_H_
#define DEBUG_DATA_WIDGET_H_

#include "HUD.h"
#include "Application.h"
#include "GameData.h"
#include "TextManager.h"

class DebugDataWidget : public Widget
{
public:
    DebugDataWidget(Application &application_, Camera &camera_, int lvlId_, Vector2<float> lvlSize_, Uint32 &frameTime_);
    virtual void update() final;
    virtual void draw(Renderer &renderer_, Camera &camera_) final;
    virtual ~DebugDataWidget() = default;

private:
    Camera &m_camera;
    TextManager &m_textManager;
    int m_lvlId;
    Vector2<float> m_lvlSize;
    Uint32 &m_frameTime;
    
};

#endif