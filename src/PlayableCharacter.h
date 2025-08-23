#ifndef PLAYABLE_CHARACTER_H_
#define PLAYABLE_CHARACTER_H_
#include "StateMachine.hpp"
#include "Core/CoreComponents.h"
#include "Core/InputResolver.h"
#include "Core/InputComparators.h"
#include "Core/StaticMapping.hpp"

enum class CharacterState : CharState {
    IDLE,
    FLOAT,
    PRERUN,
    RUN,
    RUN_RECOVERY,
    PREJUMP,
    PREJUMP_FORWARD,
    LANDING_RECOVERY,
    HARD_LANDING_RECOVERY,
    WALL_CLING,
    WALL_CLING_PREJUMP,
    ATTACK_1,
    ATTACK_1_CHAIN,
    AIR_ATTACK
};

SERIALIZE_ENUM(CharacterState, {
    ENUM_AUTO(CharacterState, IDLE),
    ENUM_AUTO(CharacterState, FLOAT),
    ENUM_AUTO(CharacterState, PRERUN),
    ENUM_AUTO(CharacterState, RUN),
    ENUM_AUTO(CharacterState, RUN_RECOVERY),
    ENUM_AUTO(CharacterState, PREJUMP),
    ENUM_AUTO(CharacterState, PREJUMP_FORWARD),
    ENUM_AUTO(CharacterState, LANDING_RECOVERY),
    ENUM_AUTO(CharacterState, HARD_LANDING_RECOVERY),
    ENUM_AUTO(CharacterState, WALL_CLING),
    ENUM_AUTO(CharacterState, WALL_CLING_PREJUMP),
    ENUM_AUTO(CharacterState, ATTACK_1),
    ENUM_AUTO(CharacterState, ATTACK_1_CHAIN),
    ENUM_AUTO(CharacterState, AIR_ATTACK),
})

template<bool REQUIRE_ALIGNMENT, bool FORCE_REALIGN, typename CMP_LEFT, typename CMP_RIGHT>
inline ORIENTATION attemptInput(ORIENTATION currentOrientation_, const InputQueue &iq_, int val_)
{
    if (REQUIRE_ALIGNMENT)
    {
        if (currentOrientation_ == ORIENTATION::RIGHT && CMP_RIGHT::check(iq_, val_) || currentOrientation_ == ORIENTATION::LEFT && CMP_LEFT::check(iq_, val_))
            return currentOrientation_;
    }
    else
    {
        auto lValid =  CMP_LEFT::check(iq_, val_);
        auto rValid = CMP_RIGHT::check(iq_, val_);

        if (rValid && currentOrientation_ == ORIENTATION::RIGHT)
            return ORIENTATION::RIGHT;

        if (lValid && currentOrientation_ == ORIENTATION::LEFT)
            return ORIENTATION::LEFT;

        if (!FORCE_REALIGN)
            return ORIENTATION::UNSPECIFIED;

        if (rValid)
            return ORIENTATION::RIGHT;
        else if (lValid)
            return ORIENTATION::LEFT;
    }

    return ORIENTATION::UNSPECIFIED;
}

template<bool REQUIRE_ALIGNMENT, bool FORCE_REALIGN, bool FORCE_TOWARDS_INPUT,
    typename CMP_LEFT, typename CMP_RIGHT,
    bool ATTEMPT_PROCEED, typename CMP_PROCEED_LEFT, typename CMP_PROCEED_RIGHT>
class PlayerState : public PhysicalState
{
public:
    PlayerState(CharacterState actionState_, StateMarker &&transitionableFrom_, ResID anim_) :
        PhysicalState(actionState_, std::move(transitionableFrom_), anim_)
    {
    }

    bool update(EntityAnywhere owner_, uint32_t currentFrame_) override
    {
        auto res = PhysicalState::update(owner_, currentFrame_);
        if (!m_lookaheadSpeedSensitivity.isEmpty())
            owner_.reg->get<ComponentDynamicCameraTarget>(owner_.idx).m_lookaheadSpeedSensitivity = m_lookaheadSpeedSensitivity[currentFrame_];

        const auto &compInput = owner_.reg->get<InputResolver>(owner_.idx);
        auto &compFallthrough = owner_.reg->get<ComponentObstacleFallthrough>(owner_.idx);
        if (m_canFallThrough[currentFrame_] && compInput.getInputQueue()[0].isInputActive(INPUT_BUTTON::DOWN))
            compFallthrough.setIgnoringObstacles();

        auto &phys = owner_.reg->get<ComponentPhysical>(owner_.idx);
        const auto &inq = compInput.getInputQueue();

        if (m_allowAirDrift)
        {
            if (InputComparatorHoldLeft::check(inq, 0))
            {
                if (phys.m_velocity.x > -2.5f)
                    phys.m_velocity.x -= 0.15f;
            }

            if (InputComparatorHoldRight::check(inq, 0))
            {
                if (phys.m_velocity.x < 2.5f)
                    phys.m_velocity.x += 0.15f;
            }

            if (phys.m_velocity.y < 0 && InputComparatorHoldUp::check(inq, 0))
            {
                if (m_parent->m_framesInState < 10)
                    phys.m_velocity.y -= 0.4f;
            }
        }

        if (!res)
            return false;

        if (!ATTEMPT_PROCEED)
            return true;

        const auto &transform = owner_.reg->get<ComponentTransform>(owner_.idx);

        auto orientation = transform.m_orientation;

        bool possibleToLeft = (!m_alignedSlopeMax.has_value() || phys.m_onSlopeWithAngle <= 0 || phys.m_onSlopeWithAngle <= m_alignedSlopeMax);
        bool possibleToRight = (!m_alignedSlopeMax.has_value() || phys.m_onSlopeWithAngle >= 0 || -phys.m_onSlopeWithAngle <= m_alignedSlopeMax);

        ORIENTATION inres = ORIENTATION::UNSPECIFIED;
        if (possibleToLeft && possibleToRight)
            inres = attemptInput<true, false, CMP_PROCEED_LEFT, CMP_PROCEED_RIGHT>(orientation, inq, 0);
        else if (possibleToLeft)
            inres = attemptInput<true, false, CMP_PROCEED_LEFT, InputComparatorFail >(orientation, inq, 0);
        else if (possibleToRight)
            inres = attemptInput<true, false, InputComparatorFail, CMP_PROCEED_RIGHT >(orientation, inq, 0);

        return inres == ORIENTATION::UNSPECIFIED;
    }

    void enter(EntityAnywhere owner_, CharState from_) override
    {
        PhysicalState::enter(owner_, from_);

        if (!m_lookaheadSpeedSensitivity.isEmpty())
            owner_.reg->get<ComponentDynamicCameraTarget>(owner_.idx).m_lookaheadSpeedSensitivity = m_lookaheadSpeedSensitivity[0];
        else
            owner_.reg->get<ComponentDynamicCameraTarget>(owner_.idx).m_lookaheadSpeedSensitivity = {1.0f, 1.0f};
    }

    ORIENTATION isPossible(EntityAnywhere owner_) const override;

    inline auto
        &setAlignedSlopeMax(float alignedSlopeMax_);

    PlayerState<REQUIRE_ALIGNMENT, FORCE_REALIGN, FORCE_TOWARDS_INPUT, CMP_LEFT, CMP_RIGHT, ATTEMPT_PROCEED, CMP_PROCEED_LEFT, CMP_PROCEED_RIGHT> 
        &setLookaheadSpeedSensitivity(TimelineProperty<Vector2<float>>&& lookaheadSpeedSensitivity_)
    {
        m_lookaheadSpeedSensitivity = std::move(lookaheadSpeedSensitivity_);
        return *this;
    }

    PlayerState<REQUIRE_ALIGNMENT, FORCE_REALIGN, FORCE_TOWARDS_INPUT, CMP_LEFT, CMP_RIGHT, ATTEMPT_PROCEED, CMP_PROCEED_LEFT, CMP_PROCEED_RIGHT> 
        &setRealignOnSwitch(bool realignOnSwitch_)
    {
        m_realignOnSwitchForInput = realignOnSwitch_;
        return *this;
    }

    PlayerState<REQUIRE_ALIGNMENT, FORCE_REALIGN, FORCE_TOWARDS_INPUT, CMP_LEFT, CMP_RIGHT, ATTEMPT_PROCEED, CMP_PROCEED_LEFT, CMP_PROCEED_RIGHT> 
        &setExtendedBuffer(uint32_t extendedBuffer_)
    {
        m_extendedBuffer = extendedBuffer_;
        return *this;
    }

    PlayerState<REQUIRE_ALIGNMENT, FORCE_REALIGN, FORCE_TOWARDS_INPUT, CMP_LEFT, CMP_RIGHT, ATTEMPT_PROCEED, CMP_PROCEED_LEFT, CMP_PROCEED_RIGHT> 
        &allowAirDrift()
    {
        m_allowAirDrift = true;
        return *this;
    }

protected:
    const Collider m_pushbox;

    std::optional<float> m_alignedSlopeMax;
    bool m_realignOnSwitchForInput = false;
    TimelineProperty<Vector2<float>> m_lookaheadSpeedSensitivity;

    uint32_t m_extendedBuffer = 0;

    bool m_allowAirDrift = false;
};


class PlayerActionFloat: public PlayerState<false, true, false, InputComparatorIdle, InputComparatorIdle, false, InputComparatorIdle, InputComparatorIdle>
{
public:
    PlayerActionFloat(ResID anim_, StateMarker transitionableFrom_) :
        PlayerState<false, true, false, InputComparatorIdle, InputComparatorIdle, false, InputComparatorIdle, InputComparatorIdle>(CharacterState::FLOAT, std::move(transitionableFrom_), anim_)
    {
    }

    virtual void enter(EntityAnywhere owner_, CharState from_) override
    {
        ParentAction::enter(owner_, from_);

        const auto &fallthrough = owner_.reg->get<ComponentObstacleFallthrough>(owner_.idx);
        auto &phys = owner_.reg->get<ComponentPhysical>(owner_.idx);
        //std::cout << fallthrough.m_ignoredObstacles.size() << std::endl;
        if (fallthrough.isIgnoringAllObstacles() && phys.peekOffset().x * phys.m_lastSlopeAngle > 0.0f)
            phys.m_velocity.y += 2.0f;
    }

    virtual bool update(EntityAnywhere owner_, uint32_t currentFrame_) override
    {
        auto res = ParentAction::update(owner_, currentFrame_);

        auto &phys = owner_.reg->get<ComponentPhysical>(owner_.idx);
        auto &trans = owner_.reg->get<ComponentTransform>(owner_.idx);

        auto total = phys.m_velocity.x + phys.m_inertia.x;
        if (total > 0)
            trans.m_orientation = ORIENTATION::RIGHT;
        else if (total < 0)
            trans.m_orientation = ORIENTATION::LEFT;

        if (phys.m_velocity.y + phys.m_inertia.y > 0)
            phys.m_gravity *= 1.3f;

        return res;
    }

    virtual void onTouchedGround(EntityAnywhere owner_) override
    {
        auto &phys = owner_.reg->get<ComponentPhysical>(owner_.idx);
        auto &wrld = owner_.reg->get<World>(owner_.idx);

        phys.m_velocity.y = 0;
        phys.m_inertia.y = 0;

        if (phys.m_calculatedOffset.y > 20.0f)
        {
            m_parent->switchCurrentState(owner_, CharacterState::HARD_LANDING_RECOVERY);
            wrld.getCamera().startShake(0, 20, 10);
        }
        else
            m_parent->switchCurrentState(owner_, CharacterState::LANDING_RECOVERY);
    }

protected:
    using ParentAction = PlayerState<false, true, false, InputComparatorIdle, InputComparatorIdle, false, InputComparatorIdle, InputComparatorIdle>;
};

class PlayerActionWallCling: public PlayerState<false, true, false, InputComparatorBufferedHoldRight, InputComparatorBufferedHoldLeft, false, InputComparatorFail, InputComparatorFail>
{
public:
    PlayerActionWallCling(ResID anim_, StateMarker transitionableFrom_, const ParticleTemplate &slideParticle_) :
        PlayerState<false, true, false, InputComparatorBufferedHoldRight, InputComparatorBufferedHoldLeft, false, InputComparatorFail, InputComparatorFail>(CharacterState::WALL_CLING, std::move(transitionableFrom_), anim_),
        m_transitionOnLeave(CharacterState::FLOAT),
        m_slideParticle(slideParticle_)
    {
        setGravity(Vector2{0.0f, 0.020f});
        setConvertVelocityOnSwitch(true, false);
    }

    virtual void enter(EntityAnywhere owner_, CharState from_) override
    {
        ParentAction::enter(owner_, from_);

        auto &physical = owner_.reg->get<ComponentPhysical>(owner_.idx);

        if (physical.m_inertia.y > 0)
            physical.m_inertia.y = 0;
        if (physical.m_velocity.y > 0)
            physical.m_velocity.y = 0;

        physical.m_velocity.x = 0;
        physical.m_inertia.x = 0;

        m_particleTimer.begin(0);
    }

    void leave(EntityAnywhere owner_, CharState to_) override;

    bool update(EntityAnywhere owner_, uint32_t currentFrame_) override
    {
        ParentAction::update(owner_, currentFrame_);

        const auto &transform = owner_.reg->get<ComponentTransform>(owner_.idx);
        auto &physical = owner_.reg->get<ComponentPhysical>(owner_.idx);
        auto &cworld = owner_.reg->get<World>(owner_.idx);
        auto pb = physical.m_pushbox + transform.m_pos;

        auto orientation = transform.m_orientation;

        auto touchedWall = (orientation == ORIENTATION::RIGHT ? 
            cworld.isWallAt(ORIENTATION::RIGHT, pb.m_topLeft + Vector2{-1, pb.m_size.y / 2}) :
            cworld.isWallAt(ORIENTATION::LEFT, pb.m_topLeft + Vector2{pb.m_size.x, pb.m_size.y / 2}));

        if (touchedWall == entt::null)
        {
            if (physical.peekRawOffset().y < 0)
                physical.m_velocity.y -= 1.0f;
            
            m_parent->switchCurrentState(owner_, m_transitionOnLeave);
        }
        else
            physical.m_onWall = touchedWall;

        if (m_particleTimer.update())
        {
            m_particleTimer.begin(8);
            auto yspd = physical.m_appliedOffset.y;
            if (yspd > 0.5f)
                spawnParticle(owner_, m_slideParticle, transform, physical, cworld, SDL_FLIP_VERTICAL);
            else if (yspd < -0.5f)
            {
                m_slideParticle.offset.y -= 30.0f;
                spawnParticle(owner_, m_slideParticle, transform, physical, cworld, SDL_FLIP_NONE);
                m_slideParticle.offset.y += 30.0f;
            }
        }

        return false;
    }

    ORIENTATION isPossible(EntityAnywhere owner_) const override
    {
        if (PhysicalState::isPossible(owner_) == ORIENTATION::UNSPECIFIED)
            return ORIENTATION::UNSPECIFIED;

        const auto &transform = owner_.reg->get<ComponentTransform>(owner_.idx);
        auto &physical = owner_.reg->get<ComponentPhysical>(owner_.idx);
        const auto &[compInput, cworld] = owner_.reg->get<InputResolver, World>(owner_.idx);

        const auto &inq = compInput.getInputQueue();
        auto pb = physical.m_pushbox + transform.m_pos;

        if (physical.m_onGround != entt::null)
            return ORIENTATION::UNSPECIFIED;

        if (physical.peekRawOffset().x >= -0.001)
        {
            if (InputComparatorBufferedHoldRight::check(inq, 0))
            {
                auto touchedWall = cworld.isWallAt(ORIENTATION::LEFT, pb.m_topLeft + Vector2{pb.m_size.x, pb.m_size.y / 2});
                if (touchedWall != entt::null) 
                {
                    physical.m_onWall = touchedWall;
                    return ORIENTATION::LEFT;
                }
            }
        }
        
        if (physical.peekRawOffset().x <= 0.001)
        {
            if (InputComparatorBufferedHoldLeft::check(inq, 0))
            {
                auto touchedWall = cworld.isWallAt(ORIENTATION::RIGHT, pb.m_topLeft + Vector2{-1, pb.m_size.y / 2});
                if (touchedWall != entt::null) 
                {
                    physical.m_onWall = touchedWall;
                    return ORIENTATION::RIGHT;
                }
            }
        }

        return ORIENTATION::UNSPECIFIED;
    }

protected:
    using ParentAction = PlayerState<false, true, false, InputComparatorBufferedHoldRight, InputComparatorBufferedHoldLeft, false, InputComparatorFail, InputComparatorFail>;
    CharacterState m_transitionOnLeave;

    ParticleTemplate m_slideParticle;
    FrameTimer<true> m_particleTimer;
};

class PlayerActionWallPrejump: public PlayerState<true, false, false, InputComparatorTapAnyLeft, InputComparatorTapAnyRight, false, InputComparatorFail, InputComparatorFail>
{
public:
    PlayerActionWallPrejump(ResID anim_, StateMarker transitionableFrom_, const ParticleTemplate &jumpParticle_) :
        PlayerState<true, false, false, InputComparatorTapAnyLeft, InputComparatorTapAnyRight, false, InputComparatorFail, InputComparatorFail>(CharacterState::WALL_CLING_PREJUMP, std::move(transitionableFrom_), anim_),
        m_jumpParticle(jumpParticle_)
    {
        setGravity(Vector2{0.0f, 0.020f});
        setConvertVelocityOnSwitch(true, true);
        setAppliedInertiaMultiplier(Vector2{0.0f, 0.0f});
    }

    void enter(EntityAnywhere owner_, CharState from_) override
    {
        ParentAction::enter(owner_, from_);

        auto &physical = owner_.reg->get<ComponentPhysical>(owner_.idx);

        if (physical.m_inertia.y > 0)
            physical.m_inertia.y = 0;
        if (physical.m_velocity.y > 0)
            physical.m_velocity.y = 0;
    }

    void leave(EntityAnywhere owner_, CharState to_) override;

    void onOutdated(EntityAnywhere owner_) override
    {
        const auto &transform = owner_.reg->get<ComponentTransform>(owner_.idx);
        auto &physical = owner_.reg->get<ComponentPhysical>(owner_.idx);
        auto &compInput = owner_.reg->get<InputResolver>(owner_.idx);
        auto &world = owner_.reg->get<World>(owner_.idx);

        const auto &inq = compInput.getInputQueue();

        Vector2<float> targetSpeed;
        int orient = static_cast<int>(transform.m_orientation);

        bool upIn = InputComparatorHoldUp::check(inq, 0);
        bool sideIn = (orient > 0 ? InputComparatorHoldRight::check(inq, 0) : InputComparatorHoldLeft::check(inq, 0));
        bool downIn = InputComparatorHoldDown::check(inq, 0);

        bool fall = false;

        if (upIn)
        {
            if (sideIn)
                targetSpeed = {orient * 1.5f, -4.5f};
            else
                targetSpeed = {orient * 0.7f, -5.0f};
        }
        else if (sideIn)
        {
            if (downIn)
                targetSpeed = {orient * 3.5f, 0.0f};
            else
                targetSpeed = {orient * 3.0f, -2.2f};
        }
        else
        {
            targetSpeed = {orient * 0.7f, 0.1f};
            fall = true;
        }

        //std::cout << targetSpeed << std::endl;

        if (targetSpeed.y >= 0)
            spawnParticle(owner_, m_jumpParticle, transform, physical, world, SDL_FLIP_VERTICAL);
        else
            spawnParticle(owner_, m_jumpParticle, transform, physical, world, SDL_FLIP_NONE);

        if (fall)
        {
            physical.m_velocity += targetSpeed;
            physical.m_inertia = {0.0f, 0.0f};
        }
        else
            physical.m_velocity += targetSpeed;

        physical.m_onWall = entt::null;

        ParentAction::onOutdated(owner_);
    }

protected:
    using ParentAction = PlayerState<true, false, false, InputComparatorTapAnyLeft, InputComparatorTapAnyRight, false, InputComparatorFail, InputComparatorFail>;

    ParticleTemplate m_jumpParticle;
};

class PlayerActionAttack1Chain: public PlayerState<false, false, true, InputComparatorTapAttack, InputComparatorTapAttack, false, InputComparatorFail, InputComparatorFail>
{
public:
    PlayerActionAttack1Chain(ResID anim_, ResID animTrace_) :
        PlayerState<false, false, true, InputComparatorTapAttack, InputComparatorTapAttack, false, InputComparatorFail, InputComparatorFail>(CharacterState::ATTACK_1_CHAIN, {}, anim_)
    {
        setLookaheadSpeedSensitivity(TimelineProperty<Vector2<float>>({
                    {0, {0.0f, 0.0f}},
                    {15, {1.0f, 1.0f}},
                    {40, {0.0f, 0.0f}},
                    {45, {1.0f, 1.0f}}
                }));
        setCanFallThrough(TimelineProperty<bool>(false));
        setExtendedBuffer(10);
        setGravity(Vector2{0.0f, 0.0f});
        setConvertVelocityOnSwitch(true, false);
        setTransitionOnLostGround(CharacterState::FLOAT);
        setMagnetLimit(TimelineProperty<unsigned int>({
                    {0, 16},
                    {15, 4}
                }));
        setUpdateMovementData(
            TimelineProperty<Vector2<float>>( 
                {
                    {0, {1.0f, 1.0f}},
                    {12, {0.1f, 1.0f}},
                    {30, {1.0f, 1.0f}},
                    {44, {0.0f, 1.0f}},
                }), // Vel mul
            TimelineProperty<Vector2<float>>( 
                {
                    {0, {0.0f, 0.0f}},
                    {9, {5.0f, 0.0f}},
                    {12, {0.0f, 0.0f}},
                    {40, {-1.5f, 0.0f}},
                    {41, {0.0f, 0.0f}}
                }),  // Dir vel mul
            TimelineProperty(Vector2<float>{0.0f, 0.0f}), // Raw vel
            TimelineProperty(Vector2<float>{1.0f, 1.0f}), // Inr mul
            TimelineProperty(Vector2<float>{0.0f, 0.0f}), // Dir inr mul
            TimelineProperty(Vector2<float>{0.0f, 0.0f})); // Raw inr
        setMulInsidePushbox(TimelineProperty<std::optional<Vector2<float>>>({
            {9, Vector2{0.2f, 1.0f}},
            {12, std::nullopt}
        }));
        setTransitionVelocityMultiplier(TimelineProperty<Vector2<float>>({
                    {0, {1.0f, 1.0f}},
                    {9, {0.2f, 1.0f}},
                    {17, {1.0f, 1.0f}}
                }));
        setHurtboxes({
            {
                HurtboxGroup(
                    {
                        {
                            {{{-6, -28}, {12, 28}}, TimelineProperty<bool>({{0, true}, {12, false}, {28, true}})},
                            {{{-30, -16}, {32, 16}}, TimelineProperty<bool>({{12, true}, {28, false}})}
                        }
                    }, HurtTrait::NORMAL
                )
            }
        });
        addHit(HitGeneration::hitPlayerChain());
        setDrag(TimelineProperty<Vector2<float>>({
            {0, {0.05f, 0.05f}},
            {8, {0.3f, 0.3f}},
            }));
        setRecoveryFrames(TimelineProperty<StateMarker>({
            {0, StateMarker{}}
            }));
        setParticlesSingle(TimelineProperty<ParticleTemplate>({
            {0, {}},
            {11, ParticleTemplate{1, Vector2<float>{5.0f, 2.0f}, animTrace_, 10,
                -1}
                .setTiePosRules(TiePosRule::TIE_TO_SOURCE)
                .setTieLifetimeRules(TieLifetimeRule::DESTROY_ON_STATE_LEAVE)
                .setNotDependOnGroundAngle()},
            {12, {}},
        }));
        setOutdatedTransition(CharacterState::IDLE, 46);
    }

    virtual bool update(EntityAnywhere owner_, uint32_t currentFrame_) override
    {
        auto updateres = PlayerState<false, false, true, InputComparatorTapAttack, InputComparatorTapAttack, false, InputComparatorFail, InputComparatorFail>::update(owner_, currentFrame_);

        if (currentFrame_ == 11)
        {
            auto &wrld = owner_.reg->get<World>(owner_.idx);
            wrld.getCamera().startShake(15, 15, 14);
        }

        return updateres;
    }
};

#endif
