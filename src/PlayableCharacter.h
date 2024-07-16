#ifndef PLAYABLE_CHARACTER_H_
#define PLAYABLE_CHARACTER_H_

#include "Action.hpp"
#include "CollisionArea.h"
#include <map>
#include "ActionCharacter.hpp"

class DebugPlayerWidget;

enum class CharacterState
{
    IDLE = 0,
    FLOAT,
    RUN,
    PREJUMP,
    PREJUMP_FORWARD,
    ATTACK1_1,
    ATTACK1_2,
    WALL_CLING,
    WALL_PREJUMP,
    NONE
};

const inline std::map<CharacterState, const char *> CharacterStateNames{
    {CharacterState::IDLE, "IDLE"},
    {CharacterState::FLOAT, "FLOAT"},
    {CharacterState::RUN, "RUN"},
    {CharacterState::PREJUMP, "PREJUMP"},
    {CharacterState::PREJUMP_FORWARD, "PREJUMP_FORWARD"},
    {CharacterState::ATTACK1_1, "ATTACK1_1"},
    {CharacterState::ATTACK1_2, "ATTACK1_2"},
    {CharacterState::WALL_CLING, "WALL_CLING"},
    {CharacterState::WALL_PREJUMP, "WALL_PREJUMP"},
};

class PlayableCharacter : public ActionCharacter<PlayableCharacter, CharacterState>
{
protected:
    using CharacterFloatAction = ActionFloat<CharacterState, PlayableCharacter&>;
    using CharacterWallClingAction = WallClingAction<CharacterState, PlayableCharacter&>;
    using CharacterWallPrejumpAction = WallClingPrejump<CharacterState, PlayableCharacter&>;

public:
    PlayableCharacter(Application &application_, const CollisionArea &cldArea_);

    virtual void update() override;
    Vector2<float> getCameraFocusPoint() const;

    CharacterGenericAction *getCurrentAction() const;
    const char *getCurrentActionName() const;

    virtual bool isIgnoringAllObstacles() override;
    virtual void cleanIgnoredObstacles() override;
    virtual bool touchedObstacleTop(int obstacleId_) override;
    virtual bool touchedObstacleSlope(int obstacleId_) override;
    virtual bool checkIgnoringObstacle(int obstacleId_) const override;

    Vector2<float> &accessPreEditVelocity();

    virtual ~PlayableCharacter() = default;

protected:
    friend CharacterGenericAction;
    friend CharacterFloatAction;
    friend CharacterWallClingAction;
    friend DebugPlayerWidget;
    friend CharacterWallPrejumpAction;

    virtual void loadAnimations(Application &application_) override;

    InputResolver m_inputResolver;

    Vector2<float> m_preEditVelocity;

    InputComparatorTapAnyDown m_fallthroughInput;
    FrameTimer<false> m_isIgnoringObstacles;

    std::array<FrameTimer<true>, 1> m_cooldowns;

    Vector2<float> m_cameraOffset;
};

#endif
