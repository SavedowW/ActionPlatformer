#ifndef DEBUG_PLAYER_WIDGET_H_
#define DEBUG_PLAYER_WIDGET_H_

#include "PlayableCharacter.h"

class DebugPlayerWidget : public Widget
{
public:
    DebugPlayerWidget(Application &application_, Camera &camera_, PlayableCharacter *pc_);
    virtual void update() final;
    virtual void draw(Renderer &renderer_, Camera &camera_) final;
    virtual ~DebugPlayerWidget() = default;

private:
    Camera &m_camera;
    TextManager &m_textManager;
    PlayableCharacter *m_pc;
    
};

#endif