#ifndef PLAYABLE_CHARACTER_H_
#define PLAYABLE_CHARACTER_H_
#include "CoreComponents.h"
#include "StateMachine.h"
#include "InputComparators.h"
#include <map>
#include <string>

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
    NONE
};

inline const std::map<CharacterState, std::string> CharacterStateNames {
    {CharacterState::IDLE, "IDLE"},
    {CharacterState::FLOAT, "FLOAT"},
    {CharacterState::PRERUN, "PRERUN"},
    {CharacterState::RUN, "RUN"},
    {CharacterState::RUN_RECOVERY, "RUN_RECOVERY"},
    {CharacterState::PREJUMP, "PREJUMP"},
    {CharacterState::PREJUMP_FORWARD, "PREJUMP_FORWARD"},
    {CharacterState::LANDING_RECOVERY, "LANDING_RECOVERY"},
    {CharacterState::HARD_LANDING_RECOVERY, "HARD_LANDING_RECOVERY"},
    {CharacterState::WALL_CLING, "WALL_CLING"},
    {CharacterState::WALL_CLING_PREJUMP, "WALL_CLING_PREJUMP"},
    {CharacterState::ATTACK_1, "ATTACK_1"}
};

template<bool REQUIRE_ALIGNMENT, bool FORCE_REALIGN>
inline ORIENTATION attemptInput(const InputComparator &cmpL_, const InputComparator &cmpR_, ORIENTATION currentOrientation_, const InputQueue &iq_, int val_)
{
    if (REQUIRE_ALIGNMENT)
    {
        if (currentOrientation_ == ORIENTATION::RIGHT && cmpR_(iq_, val_) || currentOrientation_ == ORIENTATION::LEFT && cmpL_(iq_, val_))
            return currentOrientation_;
    }
    else
    {
        auto lValid = cmpL_(iq_, val_);
        auto rValid = cmpR_(iq_, val_);

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
    PlayerState(CharacterState actionState_, StateMarker &&transitionableFrom_, int anim_) :
        PhysicalState(actionState_, CharacterStateNames.at(actionState_), std::move(transitionableFrom_), anim_)
    {
    }

    inline virtual bool update(EntityAnywhere owner_, uint32_t currentFrame_)
    {
        auto res = PhysicalState::update(owner_, currentFrame_);

        const auto &compInput = owner_.reg->get<ComponentPlayerInput>(owner_.idx);
        auto &compFallthrough = owner_.reg->get<ComponentObstacleFallthrough>(owner_.idx);
        if (compInput.m_inputResolver->getInputQueue()[0].m_inputs.at(INPUT_BUTTON::DOWN) == INPUT_BUTTON_STATE::PRESSED)
            compFallthrough.setIgnoringObstacles();

        if (!res)
            return false;

        if (!ATTEMPT_PROCEED)
            return true;

        const auto &transform = owner_.reg->get<ComponentTransform>(owner_.idx);
        const auto &physical = owner_.reg->get<ComponentPhysical>(owner_.idx);

        auto orientation = transform.m_orientation;
        const auto &inq = compInput.m_inputResolver->getInputQueue();

        bool possibleToLeft = (!m_alignedSlopeMax.has_value() || physical.m_onSlopeWithAngle <= 0 || physical.m_onSlopeWithAngle <= m_alignedSlopeMax);
        bool possibleToRight = (!m_alignedSlopeMax.has_value() || physical.m_onSlopeWithAngle >= 0 || -physical.m_onSlopeWithAngle <= m_alignedSlopeMax);

        InputComparatorFail failin;

        auto &lInput = (possibleToLeft ? static_cast<const InputComparator&>(m_cmpProcLeft) : static_cast<const InputComparator&>(failin));
        auto &rInput = (possibleToRight ? static_cast<const InputComparator&>(m_cmpProcRight) : static_cast<const InputComparator&>(failin));

        auto inres = attemptInput<true, false>(lInput, rInput, orientation, inq, 0);
        return inres == ORIENTATION::UNSPECIFIED;

    }

    inline virtual ORIENTATION isPossible(EntityAnywhere owner_) const override
    {
        if (PhysicalState::isPossible(owner_) == ORIENTATION::UNSPECIFIED)
            return ORIENTATION::UNSPECIFIED;

        const auto &transform = owner_.reg->get<ComponentTransform>(owner_.idx);
        const auto &physical = owner_.reg->get<ComponentPhysical>(owner_.idx);
        const auto &compInput = owner_.reg->get<ComponentPlayerInput>(owner_.idx);

        auto orientation = transform.m_orientation;
        const auto &inq = compInput.m_inputResolver->getInputQueue();

        bool possibleToLeft = (!m_alignedSlopeMax.has_value() || physical.m_onSlopeWithAngle <= 0 || physical.m_onSlopeWithAngle <= m_alignedSlopeMax) && (!FORCE_TOWARDS_INPUT || inq[0].m_dir.x <= 0);
        bool possibleToRight = (!m_alignedSlopeMax.has_value() || physical.m_onSlopeWithAngle >= 0 || -physical.m_onSlopeWithAngle <= m_alignedSlopeMax) && (!FORCE_TOWARDS_INPUT || inq[0].m_dir.x >= 0);

        InputComparatorFail failin;

        auto &lInput = (possibleToLeft ? static_cast<const InputComparator&>(m_cmpLeft) : static_cast<const InputComparator&>(failin));
        auto &rInput = (possibleToRight ? static_cast<const InputComparator&>(m_cmpRight) : static_cast<const InputComparator&>(failin));

        return attemptInput<REQUIRE_ALIGNMENT, FORCE_REALIGN | FORCE_TOWARDS_INPUT>(lInput, rInput, orientation, inq, 0);
    }

    inline PlayerState<REQUIRE_ALIGNMENT, FORCE_REALIGN, FORCE_TOWARDS_INPUT, CMP_LEFT, CMP_RIGHT, ATTEMPT_PROCEED, CMP_PROCEED_LEFT, CMP_PROCEED_RIGHT> 
        &setAlignedSlopeMax(float alignedSlopeMax_)
    {
        m_alignedSlopeMax = alignedSlopeMax_;
        return *this;
    }

    inline PlayerState<REQUIRE_ALIGNMENT, FORCE_REALIGN, FORCE_TOWARDS_INPUT, CMP_LEFT, CMP_RIGHT, ATTEMPT_PROCEED, CMP_PROCEED_LEFT, CMP_PROCEED_RIGHT> 
        &setRealignOnSwitch(bool realignOnSwitch_)
    {
        m_realignOnSwitchForInput = realignOnSwitch_;
        return *this;
    }

protected:
    const Collider m_pushbox;
    CMP_LEFT m_cmpLeft;
    CMP_RIGHT m_cmpRight;
    CMP_PROCEED_LEFT m_cmpProcLeft;
    CMP_PROCEED_RIGHT m_cmpProcRight;

    std::optional<float> m_alignedSlopeMax;
    bool m_realignOnSwitchForInput = false;
};


class PlayerActionFloat: public PlayerState<false, true, false, InputComparatorIdle, InputComparatorIdle, false, InputComparatorIdle, InputComparatorIdle>
{
public:
    PlayerActionFloat(int anim_, StateMarker transitionableFrom_) :
        PlayerState<false, true, false, InputComparatorIdle, InputComparatorIdle, false, InputComparatorIdle, InputComparatorIdle>(CharacterState::FLOAT, std::move(transitionableFrom_), anim_)
    {
    }

    inline virtual void enter(EntityAnywhere owner_, CharState from_) override
    {
        ParentAction::enter(owner_, from_);

        const auto &fallthrough = owner_.reg->get<ComponentObstacleFallthrough>(owner_.idx);
        auto &phys = owner_.reg->get<ComponentPhysical>(owner_.idx);
        //std::cout << fallthrough.m_ignoredObstacles.size() << std::endl;
        if (fallthrough.isIgnoringAllObstacles() && phys.getPosOffest().x * phys.m_lastSlopeAngle > 0.0f)
            phys.m_velocity.y += 2.0f;
    }

    inline virtual bool update(EntityAnywhere owner_, uint32_t currentFrame_) override
    {
        auto res = ParentAction::update(owner_, currentFrame_);

        const auto &inq = owner_.reg->get<ComponentPlayerInput>(owner_.idx).m_inputResolver->getInputQueue();
        auto &phys = owner_.reg->get<ComponentPhysical>(owner_.idx);
        auto &trans = owner_.reg->get<ComponentTransform>(owner_.idx);

        if (m_driftLeftInput(inq, 0))
        {
            if (phys.m_velocity.x > -2.5f)
                phys.m_velocity.x -= 0.15f;
        }

        if (m_driftRightInput(inq, 0))
        {
            if (phys.m_velocity.x < 2.5f)
                phys.m_velocity.x += 0.15f;
        }

        if (phys.m_velocity.y < 0 && m_driftUpInput(inq, 0))
        {
            if (m_parent->m_framesInState < 10.0f)
                phys.m_velocity.y -= 0.4f;
        }

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
    InputComparatorHoldLeft m_driftLeftInput;
    InputComparatorHoldRight m_driftRightInput;
    InputComparatorHoldUp m_driftUpInput;

};

class PlayerActionWallCling: public PlayerState<false, true, false, InputComparatorBufferedHoldRight, InputComparatorBufferedHoldLeft, false, InputComparatorFail, InputComparatorFail>
{
public:
    PlayerActionWallCling(int anim_, StateMarker transitionableFrom_, ParticleTemplate &&slideParticle_) :
        PlayerState<false, true, false, InputComparatorBufferedHoldRight, InputComparatorBufferedHoldLeft, false, InputComparatorFail, InputComparatorFail>(CharacterState::WALL_CLING, std::move(transitionableFrom_), anim_),
        m_transitionOnLeave(CharacterState::FLOAT),
        m_slideParticle(std::move(slideParticle_))
    {
        setGravity(TimelineProperty<Vector2<float>>({0.0f, 0.020f}));
        setConvertVelocityOnSwitch(true, false);
    }

    inline virtual void enter(EntityAnywhere owner_, CharState from_) override
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

    virtual void leave(EntityAnywhere owner_, CharState to_) override;

    inline virtual bool update(EntityAnywhere owner_, uint32_t currentFrame_) override
    {
        ParentAction::update(owner_, currentFrame_);

        const auto &transform = owner_.reg->get<ComponentTransform>(owner_.idx);
        auto &physical = owner_.reg->get<ComponentPhysical>(owner_.idx);
        auto &cworld = owner_.reg->get<World>(owner_.idx);
        auto pb = physical.m_pushbox + transform.m_pos;

        auto orientation = transform.m_orientation;

        auto touchedWall = (orientation == ORIENTATION::RIGHT ? 
            cworld.getTouchedWallAt(ORIENTATION::RIGHT, pb.m_center - Vector2{pb.m_halfSize.x, 0.0f}) :
            cworld.getTouchedWallAt(ORIENTATION::LEFT, pb.m_center + Vector2{pb.m_halfSize.x, 0.0f}));

        if (touchedWall == entt::null)
        {
            if (physical.getPosOffest().y < 0)
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

    inline virtual ORIENTATION isPossible(EntityAnywhere owner_) const override
    {
        if (PhysicalState::isPossible(owner_) == ORIENTATION::UNSPECIFIED)
            return ORIENTATION::UNSPECIFIED;

        const auto &transform = owner_.reg->get<ComponentTransform>(owner_.idx);
        auto &physical = owner_.reg->get<ComponentPhysical>(owner_.idx);
        const auto &compInput = owner_.reg->get<ComponentPlayerInput>(owner_.idx);
        const auto &cworld = owner_.reg->get<World>(owner_.idx);
        const auto &cobs = owner_.reg->get<ComponentObstacleFallthrough>(owner_.idx);

        const auto &inq = compInput.m_inputResolver->getInputQueue();
        auto pb = physical.m_pushbox + transform.m_pos;

        if (physical.m_onGround != entt::null)
            return ORIENTATION::UNSPECIFIED;

        if (physical.getPosOffest().x >= -0.5f)
        {
            if (m_cmpLeft(inq, 0))
            {
                auto touchedWall = cworld.getTouchedWallAt(ORIENTATION::LEFT, pb.m_center + Vector2{pb.m_halfSize.x, 0.0f});
                if (touchedWall != entt::null) 
                {
                    physical.m_onWall = touchedWall;
                    return ORIENTATION::LEFT;
                }
            }
        }
        if (physical.getPosOffest().x <= 0.5f)
        {
            if (m_cmpRight(inq, 0))
            {
                auto touchedWall = cworld.getTouchedWallAt(ORIENTATION::RIGHT, pb.m_center - Vector2{pb.m_halfSize.x, 0.0f});
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
    PlayerActionWallPrejump(int anim_, StateMarker transitionableFrom_, ParticleTemplate &&jumpParticle_) :
        PlayerState<true, false, false, InputComparatorTapAnyLeft, InputComparatorTapAnyRight, false, InputComparatorFail, InputComparatorFail>(CharacterState::WALL_CLING_PREJUMP, std::move(transitionableFrom_), anim_),
        m_jumpParticle(std::move(jumpParticle_))
    {
        setGravity(TimelineProperty<Vector2<float>>({0.0f, 0.020f}));
        setConvertVelocityOnSwitch(true, true);
        setAppliedInertiaMultiplier(TimelineProperty<Vector2<float>>({0.0f, 0.0f}));
    }

    inline virtual void enter(EntityAnywhere owner_, CharState from_) override
    {
        ParentAction::enter(owner_, from_);

        auto &physical = owner_.reg->get<ComponentPhysical>(owner_.idx);
        auto &transform = owner_.reg->get<ComponentTransform>(owner_.idx);
        const auto &cworld = owner_.reg->get<World>(owner_.idx);

        auto pb = physical.m_pushbox + transform.m_pos;

        if (physical.m_inertia.y > 0)
            physical.m_inertia.y = 0;
        if (physical.m_velocity.y > 0)
            physical.m_velocity.y = 0;
    }

    virtual void leave(EntityAnywhere owner_, CharState to_) override;

    virtual void onOutdated(EntityAnywhere owner_)
    {
        const auto &transform = owner_.reg->get<ComponentTransform>(owner_.idx);
        auto &physical = owner_.reg->get<ComponentPhysical>(owner_.idx);
        auto &compInput = owner_.reg->get<ComponentPlayerInput>(owner_.idx);
        auto &world = owner_.reg->get<World>(owner_.idx);

        const auto &inq = compInput.m_inputResolver->getInputQueue();

        Vector2<float> targetSpeed;
        int orient = static_cast<int>(transform.m_orientation);

        bool upIn = m_u(inq, 0);
        bool sideIn = (orient > 0 ? m_r(inq, 0) : m_l(inq, 0));
        bool downIn = m_d(inq, 0);

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
    InputComparatorHoldLeft m_l;
    InputComparatorHoldRight m_r;
    InputComparatorHoldUp m_u;
    InputComparatorHoldDown m_d;

    ParticleTemplate m_jumpParticle;
};

#endif
