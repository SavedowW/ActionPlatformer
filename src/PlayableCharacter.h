#ifndef PLAYABLE_CHARACTER_H_
#define PLAYABLE_CHARACTER_H_

#include "Action.hpp"
#include "CollisionArea.h"
#include <map>

class DebugPlayerWidget;

enum class CharacterState
{
    IDLE = 0,
    FLOAT,
    RUN,
    PREJUMP,
    PREJUMP_FORWARD,
    NONE
};

const inline std::map<CharacterState, const char *> CharacterStateNames{
    {CharacterState::IDLE, "IDLE"},
    {CharacterState::FLOAT, "FLOAT"},
    {CharacterState::RUN, "RUN"},
    {CharacterState::PREJUMP, "PREJUMP"},
    {CharacterState::PREJUMP_FORWARD, "PREJUMP_FORWARD"},
};

class PlayableCharacter : public Object
{
protected:
    using CharacterGenericAction = GenericAction<CharacterState, PlayableCharacter&>;
    using CharacterFloatAction = ActionFloat<CharacterState, PlayableCharacter&>;

public:
    PlayableCharacter(Application &application_, Vector2<float> pos_, const CollisionArea &cldArea_);

    virtual void update() override;
    virtual void draw(Camera &cam_) override;
    Collider getPushbox() const override;
    Vector2<float> getCameraFocusPoint() const;

    CharacterState getCurrentActionState() const;
    const char *getCurrentActionName() const;
    void switchTo(CharacterState state_);
    void switchTo(CharacterGenericAction *charAction_);
    
    void onTouchedGround();
    void onLostGround();
    bool attemptResetGround();

    bool isIgnoringAllObstacles();
    void cleanIgnoredObstacles();
    bool touchedObstacleTop(int obstacleId_);
    bool touchedObstacleBottom(int obstacleId_);
    bool touchedObstacleSlope(int obstacleId_);
    bool touchedObstacleSide(int obstacleId_);
    bool checkIgnoringObstacle(int obstacleId_) const;

    Vector2<float> &accessPreEditVelocity();
    virtual float getInertiaDrag() const override;
    virtual Vector2<float> getInertiaMultiplier() const override;

    virtual ~PlayableCharacter() = default;

protected:
    friend CharacterGenericAction;
    friend CharacterFloatAction;
    friend DebugPlayerWidget;

    CharacterGenericAction *getAction(CharacterState charState_);
    virtual void loadAnimations(Application &application_) override;
    virtual Vector2<float> getCurrentGravity() const override;

    // true if no transition happend
    bool transitionState();

    Renderer &m_renderer;

    std::map<int, std::unique_ptr<Animation>> m_animations;
    Animation *m_currentAnimation;
    InputResolver m_inputResolver;

    std::vector<std::unique_ptr<CharacterGenericAction>> m_actions;

    CharacterGenericAction *m_currentAction;

    Vector2<float> m_preEditVelocity;

    uint32_t m_framesInState = 0;
    bool isGrounded = false;

    const CollisionArea &m_collisionArea;

    InputComparatorTapAnyDown m_fallthroughInput;
    FrameTimer<false> m_isIgnoringObstacles;
    std::set<int> m_ignoredObstacles;

    std::array<FrameTimer<true>, 1> m_cooldowns;

    Vector2<float> m_cameraOffset;
};

#endif
