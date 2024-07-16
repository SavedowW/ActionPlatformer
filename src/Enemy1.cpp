#include "Enemy1.h"

Enemy1::Enemy1(Application &application_, const CollisionArea &cldArea_) :
    ActionCharacter(application_, cldArea_)
{
    AnimationManager animmgmgt = *application_.getAnimationManager();

    m_actions.push_back(
        std::unique_ptr<CharacterGenericAction>(new MobAction<Enemy1States, decltype(*this)> (
                Enemy1States::IDLE, Collider{-10, -60, 20, 60}, animmgmgt.getAnimID("Char1/run"), StateMarker{Enemy1States::NONE, {}}, *this
            )
        )
    );

    m_actions.push_back(
        std::unique_ptr<CharacterGenericAction>(
            &(new MobAction<Enemy1States, decltype(*this)> (
                Enemy1States::FLOAT, Collider{-10, -60, 20, 60}, animmgmgt.getAnimID("Char1/float"), StateMarker{Enemy1States::NONE, {}}, *this
            ))
            ->setGroundedOnSwitch(false)
            .setTransitionOnTouchedGround(Enemy1States::IDLE)
            .setGravity({{0.0f, 0.5f}})
            .setDrag(TimelineProperty<Vector2<float>>({0.0f, 0.0f}))
        )
    );

    m_currentAction = getAction(Enemy1States::FLOAT);
}

bool Enemy1::isIgnoringAllObstacles()
{
    return false;
}

void Enemy1::loadAnimations(Application &application_)
{
    AnimationManager animmgmgt = *application_.getAnimationManager();

    m_animations[animmgmgt.getAnimID("Char1/run")] = std::make_unique<Animation>(animmgmgt, animmgmgt.getAnimID("Char1/run"), LOOPMETHOD::JUMP_LOOP);
    m_animations[animmgmgt.getAnimID("Char1/float")] = std::make_unique<Animation>(animmgmgt, animmgmgt.getAnimID("Char1/float"), LOOPMETHOD::NOLOOP);

    m_currentAnimation = m_animations[animmgmgt.getAnimID("Char1/float")].get();
    m_currentAnimation->reset();
}
