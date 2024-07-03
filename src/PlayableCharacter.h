#ifndef PLAYABLE_CHARACTER_H_
#define PLAYABLE_CHARACTER_H_

#include "Object.h"

enum class CharacterState
{
    IDLE,
    FLOAT,
};

class PlayableCharacter : public Object, public InputReactor
{
public:
    PlayableCharacter(Application &application_, Vector2<float> pos_);

    virtual void update() override;
    virtual void draw(Camera &cam_) override;
    void receiveInput(EVENTS event, const float scale_) final;
    Collider getPushbox() const override;

    virtual ~PlayableCharacter() = default;

protected:
    virtual void loadAnimations(Application &application_) override;

    Renderer &m_renderer;

    std::map<int, std::unique_ptr<Animation>> m_animations;
    Animation *m_currentAnimation;
    ORIENTATION m_ownOrientation = ORIENTATION::RIGHT;
};

#endif
