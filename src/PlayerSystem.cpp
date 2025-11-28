#include "PlayerSystem.h"
#include "Core/InputResolver.h"
#include "Core/Timer.h"
#include "PlayableCharacter.hpp"
#include "StateMachine.h"
#include "ResetHandlers.h"
#include "Core/Application.h"

PlayerSystem::PlayerSystem(entt::registry &reg_) :
    m_reg(reg_),
    m_animManager(Application::instance().m_animationManager)
{
    
}

void PlayerSystem::setup(entt::entity playerId_)
{
    auto [trans, phys, inp, animrnd, sm, transreset, smreset] = m_reg.get<ComponentTransform, ComponentPhysical, InputResolver, ComponentAnimationRenderable, StateMachine,
        ComponentReset<ComponentTransform>, ComponentReset<StateMachine>>(playerId_);

    if (m_reg.all_of<ComponentSpawnLocation>(playerId_))
    {
        trans.m_pos = m_reg.get<ComponentSpawnLocation>(playerId_).m_location;
    }

    trans.m_orientation = ORIENTATION::RIGHT;

    transreset.m_defaultPos = trans.m_pos;
    transreset.m_defaultOrientation = trans.m_orientation;

    animrnd.loadAnimation(m_animManager, m_animManager.getAnimID("Char1/idle"));
    animrnd.loadAnimation(m_animManager, m_animManager.getAnimID("Char1/run"));
    animrnd.loadAnimation(m_animManager, m_animManager.getAnimID("Char1/prejump"));
    animrnd.loadAnimation(m_animManager, m_animManager.getAnimID("Char1/float"), LOOPMETHOD::NOLOOP);
    animrnd.loadAnimation(m_animManager, m_animManager.getAnimID("Char1/wall_cling"), LOOPMETHOD::NOLOOP);
    animrnd.loadAnimation(m_animManager, m_animManager.getAnimID("Char1/wall_prejump"), LOOPMETHOD::NOLOOP);
    animrnd.loadAnimation(m_animManager, m_animManager.getAnimID("Char1/prerun"), LOOPMETHOD::NOLOOP);
    animrnd.loadAnimation(m_animManager, m_animManager.getAnimID("Char1/run_recovery"), LOOPMETHOD::NOLOOP);
    animrnd.loadAnimation(m_animManager, m_animManager.getAnimID("Char1/landing_recovery"), LOOPMETHOD::NOLOOP);
    animrnd.loadAnimation(m_animManager, m_animManager.getAnimID("Char1/attack1"), LOOPMETHOD::NOLOOP);
    animrnd.loadAnimation(m_animManager, m_animManager.getAnimID("Char1/attack1_chain"), LOOPMETHOD::NOLOOP);
    animrnd.loadAnimation(m_animManager, m_animManager.getAnimID("Char1/air_attack"), LOOPMETHOD::NOLOOP);

    animrnd.m_currentAnimation = &animrnd.m_animations.at(m_animManager.getAnimID("Char1/float"));
    animrnd.m_currentAnimation->reset();

    m_animManager.preload("Char1/particles/particle_jump");
    m_animManager.preload("Char1/particles/particle_land");
    m_animManager.preload("Char1/particles/particle_run");
    m_animManager.preload("Char1/particles/particle_run_loop");
    m_animManager.preload("Char1/particles/particle_wall_jump");
    m_animManager.preload("Char1/particles/particle_wall_slide");
    m_animManager.preload("Char1/particles/attack1_trace");
    m_animManager.preload("Char1/particles/attack1_chain_trace");
    m_animManager.preload("Char1/particles/air_attack_trace");


    phys.m_pushbox = {.m_topLeft=Vector2{-7.0f, -32.0f}, .m_size=Vector2{14.0f, 32.0f}};
    phys.m_gravity = {0.0f, 0.5f};


    inp.subscribePlayer();
    inp.setInputEnabled();

    sm.addState<PlayerActionWallPrejump>(
            m_animManager.getAnimID("Char1/wall_prejump"), StateMarker{CharacterState::WALL_CLING},
            ParticleTemplate{1, Vector2<float>{-8.00f, 0.0f}, m_animManager.getAnimID("Char1/particles/particle_wall_jump"), 21,
                7}.setTiePosRules(TiePosRule::TIE_TO_WALL))
        .addTransitionOnTouchedGround(Time::fromFrames(0), CharacterState::IDLE)
        .setHurtboxes({
            {
                HurtboxGroup(
                    {
                        {
                            {.m_collider={.m_topLeft={-6, -28}, .m_size={12, 28}}, .m_timeline=TimelineProperty<Time::NS, bool>(true)}
                        }
                    }, HurtTrait::NORMAL
                )
            }
        })
        .setDrag(TimelineProperty<Time::NS, Vector2<float>>(Vector2<float>{60.0f * 60.0f, 30.0f * 60.0f}))
        .setOutdatedTransition(CharacterState::FLOAT, Time::fromFrames(3));

    sm.addState<PlayerActionWallCling>(
            m_animManager.getAnimID("Char1/wall_cling"), StateMarker{CharacterState::FLOAT},
        ParticleTemplate{1, Vector2<float>{0.0f, 25.0f}, m_animManager.getAnimID("Char1/particles/particle_wall_slide"), 21,
                7}.setTiePosRules(TiePosRule::TIE_TO_WALL))
        .setDrag(TimelineProperty<Time::NS, Vector2<float>>({
            {Time::fromFrames(0), {60.0f * 60.0f, 12.0f * 60.0f}},
            {Time::fromFrames(1), {60.0f * 60.0f, 24.0f * 60.0f}},
            }))
        .addTransitionOnTouchedGround(Time::fromFrames(0), CharacterState::IDLE)
        .setHurtboxes({
            {
                HurtboxGroup(
                    {
                        {
                            {.m_collider={.m_topLeft={-6, -28}, .m_size={12, 28}}, .m_timeline=TimelineProperty<Time::NS, bool>(true)}
                        }
                    }, HurtTrait::NORMAL
                )
            }
        });

    sm.addState<PlayerState<false, true, false, InputComparatorTapUpLeft, InputComparatorTapUpRight, true, InputComparatorIdle, InputComparatorIdle>>(
            CharacterState::PREJUMP_FORWARD, StateMarker{CharacterState::IDLE, CharacterState::PRERUN, CharacterState::RUN, CharacterState::RUN_RECOVERY, CharacterState::LANDING_RECOVERY}, m_animManager.getAnimID("Char1/prejump"))
        .setAlignedSlopeMax(0.5f)
        .setGravity(Vector2<float>{0.0f, 0.0f})
        .setConvertVelocityOnSwitch(true, false)
        .setTransitionOnLostGround(CharacterState::FLOAT)
        .setMagnetLimit(TimelineProperty<Time::NS, unsigned int>(4))
        .setUpdateMovementData(
            TimelineProperty<Time::NS, Vector2<float>>(Vector2<float>{1.0f, 1.0f}), // Vel mul
            TimelineProperty<Time::NS, Vector2<float>>(
                {
                    {Time::fromFrames(0), {0.0f, 0.0f}},
                    {Time::fromFrames(1), {1.5f * 60.0f, 0.0f}},
                }), // Dir vel mul
            TimelineProperty<Time::NS, Vector2<float>>(
                {
                    {Time::fromFrames(0), {0.0f, 0.0f}},
                    {Time::fromFrames(1), {0.0f, -4.0f * 60.0f}},
                }), // Raw vel
            TimelineProperty<Time::NS, Vector2<float>>(
                {
                    {Time::fromFrames(0), {1.0f, 1.0f}},
                    {Time::fromFrames(1), {0.5f, 1.0f}}
                }), // Inr mul
            TimelineProperty<Time::NS, Vector2<float>>(Vector2<float>{0.0f, 0.0f}), // Dir inr mul
            TimelineProperty<Time::NS, Vector2<float>>(Vector2<float>{0.0f, 0.0f})) // Raw inr
        .setDrag(TimelineProperty<Time::NS, Vector2<float>>(Vector2<float>{0.0f, 0.0f}))
        .setAppliedInertiaMultiplier(TimelineProperty<Time::NS, Vector2<float>>(Vector2<float>{0.0f, 0.0f}))
        .setHurtboxes({
            {
                HurtboxGroup(
                    {
                        {
                            {.m_collider={.m_topLeft={-6, -28}, .m_size={12, 28}}, .m_timeline=TimelineProperty<Time::NS, bool>(true)}
                        }
                    }, HurtTrait::NORMAL
                )
            }
        })
        .setOutdatedTransition(CharacterState::FLOAT, Time::fromFrames(1))
        .setParticlesSingle(TimelineProperty<Time::NS, ParticleTemplate>({
            {Time::fromFrames(0), {}},
            {Time::fromFrames(1), ParticleTemplate{1, Vector2<float>{0.0f, 0.0f}, m_animManager.getAnimID("Char1/particles/particle_jump"), 22,
                7}.setTiePosRules(TiePosRule::TIE_TO_GROUND)},
            {Time::fromFrames(2), {}},
            }));

    sm.addState<PlayerState<false, false, false, InputComparatorTapUp, InputComparatorTapUp, true, InputComparatorIdle, InputComparatorIdle>>(
            CharacterState::PREJUMP, StateMarker{CharacterState::IDLE, CharacterState::PRERUN, CharacterState::RUN, CharacterState::RUN_RECOVERY, CharacterState::LANDING_RECOVERY}, m_animManager.getAnimID("Char1/prejump"))
        .setGravity(Vector2<float>{0.0f, 0.0f})
        .setConvertVelocityOnSwitch(true, false)
        .setTransitionOnLostGround(CharacterState::FLOAT)
        .setMagnetLimit(TimelineProperty<Time::NS, unsigned int>(4))
        .setUpdateMovementData(
            TimelineProperty<Time::NS, Vector2<float>>(Vector2<float>{1.0f, 1.0f}), // Vel mul
            TimelineProperty<Time::NS, Vector2<float>>(Vector2<float>{0.0f, 0.0f}), // Dir vel mul
            TimelineProperty<Time::NS, Vector2<float>>(
                {
                    {Time::fromFrames(0), {0.0f, 0.0f}},
                    {Time::fromFrames(1), {0.0f, -4.5f * 60.0f}},
                }), // Raw vel
            TimelineProperty<Time::NS, Vector2<float>>(
                {
                    {Time::fromFrames(0), {1.0f, 1.0f}},
                    {Time::fromFrames(1), {0.5f, 1.0f}}
                }), // Inr mul
            TimelineProperty<Time::NS, Vector2<float>>(Vector2<float>{0.0f, 0.0f}), // Dir inr mul
            TimelineProperty<Time::NS, Vector2<float>>(Vector2<float>{0.0f, 0.0f})) // Raw inr
        .setDrag(TimelineProperty<Time::NS, Vector2<float>>(Vector2<float>{0.0f, 0.0f}))
        .setAppliedInertiaMultiplier(TimelineProperty<Time::NS, Vector2<float>>(Vector2<float>{0.0f, 0.0f}))
        .setUpdateSpeedLimitData(
            TimelineProperty<Time::NS, Vector2<float>>(),
            TimelineProperty<Time::NS, Vector2<float>>(Vector2<float>{9999.9f, 4.0f}))
        .setHurtboxes({
            {
                HurtboxGroup(
                    {
                        {
                            {{{-6, -28}, {12, 28}}, TimelineProperty<Time::NS, bool>(true)}
                        }
                    }, HurtTrait::NORMAL
                )
            }
        })
        .setOutdatedTransition(CharacterState::FLOAT, Time::fromFrames(1))
        .setParticlesSingle(TimelineProperty<Time::NS, ParticleTemplate>({
            {Time::fromFrames(0), {}},
            {Time::fromFrames(1), ParticleTemplate{1, Vector2<float>{0.0f, 0.0f}, m_animManager.getAnimID("Char1/particles/particle_jump"), 22,
                7}.setTiePosRules(TiePosRule::TIE_TO_GROUND)},
            {Time::fromFrames(2), {}},
            }));

    sm.addState<PlayerState<false, false, true, InputComparatorTapAttack, InputComparatorTapAttack, false, InputComparatorFail, InputComparatorFail>>(
            CharacterState::ATTACK_1, StateMarker{CharacterState::IDLE, CharacterState::PRERUN, CharacterState::RUN, CharacterState::RUN_RECOVERY, CharacterState::LANDING_RECOVERY}, m_animManager.getAnimID("Char1/attack1"))
        .setLookaheadSpeedSensitivity(TimelineProperty<Time::NS, Vector2<float>>({
                    {Time::fromFrames(0), {0.0f, 1.0f}},
                    {Time::fromFrames(8), {1.0f, 1.0f}}
                }))
        .setCanFallThrough(TimelineProperty<Time::NS, bool>(false))
        .setGravity(Vector2<float>{0.0f, 0.0f})
        .setConvertVelocityOnSwitch(true, false)
        .setTransitionOnLostGround(CharacterState::FLOAT)
        .setMagnetLimit(TimelineProperty<Time::NS, unsigned int>({
                    {Time::fromFrames(0), 10},
                    {Time::fromFrames(10), 4}
                }))
        .setUpdateMovementData(
            TimelineProperty<Time::NS, Vector2<float>>( 
                {
                    {Time::fromFrames(0), {1.0f, 1.0f}},
                    {Time::fromFrames(5), {0.5f, 0.0f}}
                }), // Vel mul
            TimelineProperty<Time::NS, Vector2<float>>( 
                {
                    {Time::fromFrames(0), {0.0f, 0.0f}},
                    {Time::fromFrames(2), {2.0f * 60.0f, 0.0f}},
                    {Time::fromFrames(5), {0.0f, 0.0f}}
                }),  // Dir vel mul
            TimelineProperty<Time::NS, Vector2<float>>(Vector2<float>{0.0f, 0.0f}), // Raw vel
            TimelineProperty<Time::NS, Vector2<float>>(Vector2<float>{1.0f, 1.0f}), // Inr mul
            TimelineProperty<Time::NS, Vector2<float>>(Vector2<float>{0.0f, 0.0f}), // Dir inr mul
            TimelineProperty<Time::NS, Vector2<float>>(Vector2<float>{0.0f, 0.0f})) // Raw inr
        .setMulInsidePushbox(TimelineProperty<Time::NS, std::optional<Vector2<float>>>({
            {Time::fromFrames(2), Vector2<float>{0.2f, 1.0f}},
            {Time::fromFrames(5), std::nullopt}
        }))
        .setTransitionVelocityMultiplier(TimelineProperty<Time::NS, Vector2<float>>({
            {Time::fromFrames(0), {1.0f, 1.0f}},
            {Time::fromFrames(2), {0.2f, 1.0f}},
            {Time::fromFrames(10), {1.0f, 1.0f}}
        }))
        .setHurtboxes({
            {
                HurtboxGroup(
                    {
                        {
                            {{{-6, -28}, {12, 28}}, TimelineProperty<Time::NS, bool>(true)}
                        }
                    }, HurtTrait::NORMAL
                )
            }
        })
        .addHit(HitGeneration::hitPlayerLight())
        .setDrag(TimelineProperty<Time::NS, Vector2<float>>({
            {Time::fromFrames(0), {3.0f * 60.0f, 3.0f * 60.0f}},
            {Time::fromFrames(8), {18.0f * 60.0f, 18.0f * 60.0f}},
            }))
        .setRecoveryFrames(TimelineProperty<Time::NS, StateMarker>({
            {Time::fromFrames(0), {}},
            {Time::fromFrames(16), {CharacterState::ATTACK_1_CHAIN}},
            {Time::fromFrames(25), {CharacterState::ATTACK_1_CHAIN, CharacterState::PREJUMP, CharacterState::PRERUN}}
            }))
        .setParticlesSingle(TimelineProperty<Time::NS, ParticleTemplate>({
            {Time::fromFrames(0), {}},
            {Time::fromFrames(4), ParticleTemplate{1, Vector2<float>{3.0f, 3.0f}, m_animManager.getAnimID("Char1/particles/attack1_trace"), 10,
                3}
                .setTiePosRules(TiePosRule::TIE_TO_SOURCE)
                .setTieLifetimeRules(TieLifetimeRule::DESTROY_ON_STATE_LEAVE)
                .setNotDependOnGroundAngle()},
            {Time::fromFrames(5), {}},
        }))
        .setOutdatedTransition(CharacterState::IDLE, Time::fromFrames(30));

    sm.addState<PlayerActionAttack1Chain>(
            m_animManager.getAnimID("Char1/attack1_chain"), m_animManager.getAnimID("Char1/particles/attack1_chain_trace"));

    sm.addState<PlayerState<true, false, false, InputComparatorFail, InputComparatorFail, true, InputComparatorHoldLeft, InputComparatorHoldRight>>(
            CharacterState::RUN, StateMarker{}, m_animManager.getAnimID("Char1/run"))
        .setGravity(Vector2<float>{0.0f, 0.0f})
        .setUpdateMovementData(
            TimelineProperty<Time::NS, Vector2<float>>(Vector2<float>{1.0f, 1.0f}), // Vel mul
            TimelineProperty<Time::NS, Vector2<float>>( 
                {
                    {Time::fromFrames(0), {0.4f * 60.0f, 0.0f}},
                    {Time::fromFrames(5), {0.6f * 60.0f, 0.0f}},
                }),  // Dir vel mul
            TimelineProperty<Time::NS, Vector2<float>>(Vector2<float>{0.0f, 0.0f}), // Raw vel
            TimelineProperty<Time::NS, Vector2<float>>(Vector2<float>{1.0f, 1.0f}), // Inr mul
            TimelineProperty<Time::NS, Vector2<float>>(Vector2<float>{0.0f, 0.0f}), // Dir inr mul
            TimelineProperty<Time::NS, Vector2<float>>(Vector2<float>{0.0f, 0.0f})) // Raw inr
        .setUpdateSpeedLimitData(
            TimelineProperty<Time::NS, Vector2<float>>(Vector2<float>{150.0f, 0.0f}),
            TimelineProperty<Time::NS, Vector2<float>>(Vector2<float>{9999.9f, 0.0f}))
        .setTransitionOnLostGround(CharacterState::FLOAT)
        .setMagnetLimit(TimelineProperty<Time::NS, unsigned int>(4))
        .setHurtboxes({
            {
                HurtboxGroup(
                    {
                        {
                            {.m_collider={.m_topLeft={-6, -28}, .m_size={12, 28}}, .m_timeline=TimelineProperty<Time::NS, bool>(true)}
                        }
                    }, HurtTrait::NORMAL
                )
            }
        })
        .setCanFallThrough(TimelineProperty<Time::NS, bool>(true))
        .setParticlesSingle(TimelineProperty<Time::NS, ParticleTemplate>({
            {Time::fromFrames(0), {}},
            {Time::fromFrames(1), ParticleTemplate{1, Vector2<float>{-10.0f, 0.0f}, m_animManager.getAnimID("Char1/particles/particle_run"), 26,
                7}.setTiePosRules(TiePosRule::TIE_TO_GROUND)},
            {Time::fromFrames(2), {}},
            }))
        .setParticlesLoopable(TimelineProperty<Time::NS, ParticleTemplate>({
            {Time::fromFrames(0), {}},
            {Time::fromFrames(5), ParticleTemplate{1, Vector2<float>{-13.0f, 0.0f}, m_animManager.getAnimID("Char1/particles/particle_run_loop"), 21,
                7}.setTiePosRules(TiePosRule::TIE_TO_GROUND)},
            {Time::fromFrames(6), {}},
            {Time::fromFrames(30), ParticleTemplate{1, Vector2<float>{-13.0f, 0.0f}, m_animManager.getAnimID("Char1/particles/particle_run_loop"), 21,
                7}.setTiePosRules(TiePosRule::TIE_TO_GROUND)},
            {Time::fromFrames(31), {}},
            }), Time::fromFrames(50));

    sm.addState<PlayerState<false, true, false, InputComparatorHoldLeft, InputComparatorHoldRight, true, InputComparatorHoldLeft, InputComparatorHoldRight>>(
            CharacterState::PRERUN, StateMarker{CharacterState::IDLE, CharacterState::RUN, CharacterState::LANDING_RECOVERY}, m_animManager.getAnimID("Char1/prerun"))
        .setGravity(Vector2<float>{0.0f, 0.0f})
        .setConvertVelocityOnSwitch(true, false)
        .setTransitionOnLostGround(CharacterState::FLOAT)
        .setMagnetLimit(TimelineProperty<Time::NS, unsigned int>(4))
        .setUpdateMovementData(
            TimelineProperty<Time::NS, Vector2<float>>(Vector2<float>{1.0f, 1.0f}), // Vel mul
            TimelineProperty<Time::NS, Vector2<float>>( 
                {
                    {Time::fromFrames(0), {0.0f, 0.0f}},
                    {Time::fromFrames(3), {0.3f * 60.0f, 0.0f}},
                }),  // Dir vel mul
            TimelineProperty<Time::NS, Vector2<float>>(Vector2<float>{0.0f, 0.0f}), // Raw vel
            TimelineProperty<Time::NS, Vector2<float>>(Vector2<float>{1.0f, 1.0f}), // Inr mul
            TimelineProperty<Time::NS, Vector2<float>>(Vector2<float>{0.0f, 0.0f}), // Dir inr mul
            TimelineProperty<Time::NS, Vector2<float>>(Vector2<float>{0.0f, 0.0f})) // Raw inr
        .setUpdateSpeedLimitData(
            TimelineProperty<Time::NS, Vector2<float>>(Vector2<float>{150.0f, 0.0f}),
            TimelineProperty<Time::NS, Vector2<float>>(Vector2<float>{9999.9f, 0.0f}))
        .setHurtboxes({
            {
                HurtboxGroup(
                    {
                        {
                            {{{-6, -28}, {12, 28}}, TimelineProperty<Time::NS, bool>(true)}
                        }
                    }, HurtTrait::NORMAL
                )
            }
        })
        .setOutdatedTransition(CharacterState::RUN, Time::fromFrames(5));

    sm.addState<PlayerState<true, false, false, InputComparatorIdle, InputComparatorIdle, false, InputComparatorIdle, InputComparatorIdle>>(
            CharacterState::RUN_RECOVERY, StateMarker{CharacterState::PRERUN, CharacterState::RUN}, m_animManager.getAnimID("Char1/run_recovery"))
        .setGravity(Vector2<float>{0.0f, 0.0f})
        .setDrag(Vector2<float>{30.0f * 60.0f, 0.0f})
        .setConvertVelocityOnSwitch(true, false)
        .setTransitionOnLostGround(CharacterState::FLOAT)
        .setMagnetLimit(TimelineProperty<Time::NS, unsigned int>(4))
        .setUpdateMovementData(
            TimelineProperty<Time::NS, Vector2<float>>(Vector2<float>{1.0f, 1.0f}), // Vel mul
            TimelineProperty<Time::NS, Vector2<float>>(Vector2<float>{0.0f, 0.0f}),  // Dir vel mul
            TimelineProperty<Time::NS, Vector2<float>>(Vector2<float>{0.0f, 0.0f}), // Raw vel
            TimelineProperty<Time::NS, Vector2<float>>(Vector2<float>{1.0f, 1.0f}), // Inr mul
            TimelineProperty<Time::NS, Vector2<float>>(Vector2<float>{0.0f, 0.0f}), // Dir inr mul
            TimelineProperty<Time::NS, Vector2<float>>(Vector2<float>{0.0f, 0.0f})) // Raw inr
        .setUpdateSpeedLimitData(
            TimelineProperty<Time::NS, Vector2<float>>(Vector2<float>{2.5f * 60.0f, 0.0f}),
            TimelineProperty<Time::NS, Vector2<float>>(Vector2<float>{9999.9f, 0.0f}))
        .setHurtboxes({
            {
                HurtboxGroup(
                    {
                        {
                            {{{-6, -28}, {12, 28}}, TimelineProperty<Time::NS, bool>(true)}
                        }
                    }, HurtTrait::NORMAL
                )
            }
        })
        .setOutdatedTransition(CharacterState::IDLE, Time::fromFrames(9));

    sm.addState<PlayerState<false, false, false, InputComparatorIdle, InputComparatorIdle, false, InputComparatorIdle, InputComparatorIdle>>(
            CharacterState::IDLE, StateMarker{}, m_animManager.getAnimID("Char1/idle"))
        .setGravity(Vector2<float>{0.0f, 0.0f})
        .setDrag(TimelineProperty<Time::NS, Vector2<float>>({{Time::fromFrames(0), Vector2{6.0f * 60.0f, 6.0f * 60.0f}}, {Time::fromFrames(3), Vector2{30.0f * 60.0f, 30.0f * 60.0f}}}))
        .setConvertVelocityOnSwitch(true, false)
        .setTransitionOnLostGround(CharacterState::FLOAT)
        .setMagnetLimit(TimelineProperty<Time::NS, unsigned int>(4))
        .setHurtboxes({
            {
                HurtboxGroup(
                    {
                        {
                            {{{-6, -28}, {12, 28}}, TimelineProperty<Time::NS, bool>(true)}
                        }
                    }, HurtTrait::NORMAL
                )
            }
        })
        .setCanFallThrough(TimelineProperty<Time::NS, bool>(true))
        .setUpdateSpeedLimitData(
            TimelineProperty<Time::NS, Vector2<float>>(Vector2<float>{9999.9f, 0.0f}),
            TimelineProperty<Time::NS, Vector2<float>>(Vector2<float>{9999.9f, 0.0f}));

    sm.addState<PlayerState<true, false, false, InputComparatorFail, InputComparatorFail, false, InputComparatorIdle, InputComparatorIdle>>(
            CharacterState::LANDING_RECOVERY, StateMarker{}, m_animManager.getAnimID("Char1/landing_recovery"))
        .setGravity(Vector2<float>{0.0f, 0.0f})
        .setConvertVelocityOnSwitch(true, false)
        .setTransitionOnLostGround(CharacterState::FLOAT)
        .setMagnetLimit(TimelineProperty<Time::NS, unsigned int>(4))
        .setHurtboxes({
            {
                HurtboxGroup(
                    {
                        {
                            {.m_collider={.m_topLeft={-6, -28}, .m_size={12, 28}}, .m_timeline=TimelineProperty<Time::NS, bool>(true)}
                        }
                    }, HurtTrait::NORMAL
                )
            }
        })
        .setOutdatedTransition(CharacterState::IDLE, Time::fromFrames(14))
        .setParticlesSingle(TimelineProperty<Time::NS, ParticleTemplate>({
            {Time::fromFrames(0), ParticleTemplate{1, Vector2<float>{0.0f, 0.0f}, m_animManager.getAnimID("Char1/particles/particle_land"), 36,
                7}.setTiePosRules(TiePosRule::TIE_TO_GROUND)},
            {Time::fromFrames(1), {}},
            }));

    sm.addState<PlayerState<true, false, false, InputComparatorFail, InputComparatorFail, false, InputComparatorIdle, InputComparatorIdle>>(
            CharacterState::HARD_LANDING_RECOVERY, StateMarker{}, m_animManager.getAnimID("Char1/landing_recovery"))
        .setGravity(Vector2<float>{0.0f, 0.0f})
        .setConvertVelocityOnSwitch(true, false)
        .setTransitionOnLostGround(CharacterState::FLOAT)
        .setMagnetLimit(TimelineProperty<Time::NS, unsigned int>(4))
        .setHurtboxes({
            {
                HurtboxGroup(
                    {
                        {
                            {.m_collider={.m_topLeft={-6, -28}, .m_size={12, 28}}, .m_timeline=TimelineProperty<Time::NS, bool>(true)}
                        }
                    }, HurtTrait::NORMAL
                )
            }
        })
        .setOutdatedTransition(CharacterState::IDLE, Time::fromFrames(14))
        .setParticlesSingle(TimelineProperty<Time::NS, ParticleTemplate>({
            {Time::fromFrames(0), ParticleTemplate{1, Vector2<float>{0.0f, 0.0f}, m_animManager.getAnimID("Char1/particles/particle_land"), 36,
                7}.setTiePosRules(TiePosRule::TIE_TO_GROUND)},
            {Time::fromFrames(1), {}},
            }));

    sm.addState<PlayerState<false, false, true, InputComparatorTapAttack, InputComparatorTapAttack, false, InputComparatorFail, InputComparatorFail>>(
            CharacterState::AIR_ATTACK, StateMarker{CharacterState::FLOAT}, m_animManager.getAnimID("Char1/air_attack"))
        .allowAirDrift()
        .setLookaheadSpeedSensitivity(TimelineProperty<Time::NS, Vector2<float>>({
                    {Time::fromFrames(0), {0.1f, 1.0f}},
                    {Time::fromFrames(5), {0.2f, 1.0f}},
                    {Time::fromFrames(10), {0.3f, 1.0f}},
                    {Time::fromFrames(20), {1.0f, 1.0f}}
                }))
        .setCanFallThrough(TimelineProperty<Time::NS, bool>(false))
        .setGravity(Vector2<float>{0.0f, 30.0f * 60.0f})
        .setConvertVelocityOnSwitch(false, true)
        .setMagnetLimit(TimelineProperty<Time::NS, unsigned int>(0))
        .setNoLanding(TimelineProperty<Time::NS, bool>(false))
        .setHurtboxes({
            {
                HurtboxGroup(
                    {
                        {
                            {.m_collider={.m_topLeft={-6, -28}, .m_size={12, 28}}, .m_timeline=TimelineProperty<Time::NS, bool>(true)}
                        }
                    }, HurtTrait::NORMAL
                )
            }
        })
        .addTransitionOnTouchedGround(Time::fromFrames(0), CharacterState::HARD_LANDING_RECOVERY)
        .addTransitionOnTouchedGround(Time::fromFrames(26), CharacterState::LANDING_RECOVERY)
        .addHit(HitGeneration::hitPlayerAirAttack())
        .setDrag(TimelineProperty<Time::NS, Vector2<float>>(Vector2<float>{0.0f, 0.0f}))
        .setParticlesSingle(TimelineProperty<Time::NS, ParticleTemplate>({
            {Time::fromFrames(0), {}},
            {Time::fromFrames(7), ParticleTemplate{1, Vector2<float>{4.0f, 42.0f}, m_animManager.getAnimID("Char1/particles/air_attack_trace"), 14,
                3}
                .setTiePosRules(TiePosRule::TIE_TO_SOURCE)
                .setTieLifetimeRules(TieLifetimeRule::DESTROY_ON_STATE_LEAVE)
                .setNotDependOnGroundAngle()},
            {Time::fromFrames(8), {}},
        }))
        .setOutdatedTransition(CharacterState::FLOAT, Time::fromFrames(35));

    sm.addState<PlayerActionFloat>(
            m_animManager.getAnimID("Char1/float"), StateMarker{})
        .allowAirDrift()
        .setLookaheadSpeedSensitivity(TimelineProperty<Time::NS, Vector2<float>>({
                    {Time::fromFrames(0), {0.5f, 0.5f}},
                    {Time::fromFrames(6), {1.0f, 1.0f}}
                }))
        .setGravity(Vector2<float>{0.0f, 30.0f * 60.0f})
        .setDrag(TimelineProperty<Time::NS, Vector2<float>>(Vector2<float>{0.0f, 0.0f}))
        .setNoLanding(TimelineProperty<Time::NS, bool>({{Time::fromFrames(0), true}, {Time::fromFrames(4), false}}))
        .setHurtboxes({
            {
                HurtboxGroup(
                    {
                        {
                            {.m_collider={.m_topLeft={-6, -28}, .m_size={12, 28}}, .m_timeline=TimelineProperty<Time::NS, bool>(true)}
                        }
                    }, HurtTrait::NORMAL
                )
            }
        })
        .setConvertVelocityOnSwitch(false, true);

    sm.setInitialState(CharacterState::FLOAT);
    smreset.m_defaultStates = {static_cast<CharState>(CharacterState::FLOAT)}; // TODO: allow any type
}

void PlayerSystem::update()
{
}
