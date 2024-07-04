#ifndef PLAYABLE_CHARACTER_H_
#define PLAYABLE_CHARACTER_H_

#include "Action.hpp"
#include <map>

enum class CharacterState
{
    IDLE = 0,
    FLOAT,
    RUN,
    NONE
};

const inline std::map<CharacterState, const char *> CharacterStateNames{
    {CharacterState::IDLE, "IDLE"},
    {CharacterState::FLOAT, "FLOAT"},
    {CharacterState::RUN, "RUN"}
};

class PlayableCharacter : public Object
{
protected:
    using CharacterGenericAction = GenericAction<CharacterState, PlayableCharacter&>;

public:
    PlayableCharacter(Application &application_, Vector2<float> pos_);

    virtual void update() override;
    virtual void draw(Camera &cam_) override;
    Collider getPushbox() const override;

    CharacterState getCurrentActionState() const;
    const char *getCurrentActionName() const;
    void switchTo(CharacterState state_);
    void switchTo(CharacterGenericAction *charAction_);
    
    void onTouchedGround();

    Vector2<float> &accessPreEditVelocity();

    virtual ~PlayableCharacter() = default;

protected:
    friend CharacterGenericAction;

    CharacterGenericAction *getAction(CharacterState charState_);
    virtual void loadAnimations(Application &application_) override;

    void transitionState();

    Renderer &m_renderer;

    std::map<int, std::unique_ptr<Animation>> m_animations;
    Animation *m_currentAnimation;
    InputResolver m_inputResolver;

    std::vector<std::unique_ptr<CharacterGenericAction>> m_actions;

    CharacterGenericAction *m_currentAction;

    Vector2<float> m_preEditVelocity;
};

#endif
