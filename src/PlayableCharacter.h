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
    InputResolver m_inputResolver;

    std::vector<std::unique_ptr<GenericAction>> m_actions;
};

#endif
