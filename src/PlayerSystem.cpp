#include "PlayerSystem.h"
#include "PlayableCharacter.h"
#include "ResetHandlers.h"
#include "Application.h"

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

    sm.addState(std::unique_ptr<GenericState>(
        &(new PlayerActionWallPrejump(
            m_animManager.getAnimID("Char1/wall_prejump"), {CharacterState::NONE, {CharacterState::WALL_CLING}},
            ParticleTemplate{1, Vector2<float>{-8.00f, 0.0f}, m_animManager.getAnimID("Char1/particles/particle_wall_jump"), 21,
                7}.setTiePosRules(TiePosRule::TIE_TO_WALL)))
        ->addTransitionOnTouchedGround(0, CharacterState::IDLE)
        .setHurtboxes({
            {
                HurtboxGroup(
                    {
                        {
                            {.m_collider={.m_topLeft={-6, -28}, .m_size={12, 28}}, .m_timeline=TimelineProperty<bool>(true)}
                        }
                    }, HurtTrait::NORMAL
                )
            }
        })
        .setDrag(TimelineProperty(Vector2<float>{1.0f, 0.5f}))
        .setOutdatedTransition(CharacterState::FLOAT, 3)
    ));

    sm.addState(std::unique_ptr<GenericState>(
        &(new PlayerActionWallCling(
            m_animManager.getAnimID("Char1/wall_cling"), {CharacterState::NONE, {CharacterState::FLOAT}},
        ParticleTemplate{1, Vector2<float>{0.0f, 25.0f}, m_animManager.getAnimID("Char1/particles/particle_wall_slide"), 21,
                7}.setTiePosRules(TiePosRule::TIE_TO_WALL)))
        ->setDrag(TimelineProperty<Vector2<float>>({
            {0, {1.0f, 0.2f}},
            {1, {1.0f, 0.4f}},
            }))
        .addTransitionOnTouchedGround(0, CharacterState::IDLE)
        .setHurtboxes({
            {
                HurtboxGroup(
                    {
                        {
                            {{{-6, -28}, {12, 28}}, TimelineProperty<bool>(true)}
                        }
                    }, HurtTrait::NORMAL
                )
            }
        })
    ));


    sm.addState(std::unique_ptr<GenericState>(
        &(new PlayerState<false, true, false, InputComparatorTapUpLeft, InputComparatorTapUpRight, true, InputComparatorIdle, InputComparatorIdle>(
            CharacterState::PREJUMP_FORWARD, {CharacterState::NONE, {CharacterState::IDLE, CharacterState::PRERUN, CharacterState::RUN, CharacterState::RUN_RECOVERY, CharacterState::LANDING_RECOVERY}}, m_animManager.getAnimID("Char1/prejump")))
        ->setAlignedSlopeMax(0.5f)
        .setGravity(Vector2<float>{0.0f, 0.0f})
        .setConvertVelocityOnSwitch(true, false)
        .setTransitionOnLostGround(CharacterState::FLOAT)
        .setMagnetLimit(TimelineProperty<unsigned int>(4))
        .setUpdateMovementData(
            TimelineProperty(Vector2<float>{1.0f, 1.0f}), // Vel mul
            TimelineProperty<Vector2<float>>(
                {
                    {0, {0.0f, 0.0f}},
                    {1, {1.5f, 0.0f}},
                }), // Dir vel mul
            TimelineProperty<Vector2<float>>(
                {
                    {0, {0.0f, 0.0f}},
                    {1, {0.0f, -4.0f}},
                }), // Raw vel
            TimelineProperty<Vector2<float>>(
                {
                    {0, {1.0f, 1.0f}},
                    {1, {0.5f, 1.0f}}
                }), // Inr mul
            TimelineProperty(Vector2<float>{0.0f, 0.0f}), // Dir inr mul
            TimelineProperty(Vector2<float>{0.0f, 0.0f})) // Raw inr
        .setDrag(TimelineProperty(Vector2<float>{0.0f, 0.0f}))
        .setAppliedInertiaMultiplier(TimelineProperty(Vector2<float>{0.0f, 0.0f}))
        .setHurtboxes({
            {
                HurtboxGroup(
                    {
                        {
                            {{{-6, -28}, {12, 28}}, TimelineProperty<bool>(true)}
                        }
                    }, HurtTrait::NORMAL
                )
            }
        })
        .setOutdatedTransition(CharacterState::FLOAT, 1)
        .setParticlesSingle(TimelineProperty<ParticleTemplate>({
            {0, {}},
            {1, ParticleTemplate{1, Vector2<float>{0.0f, 0.0f}, m_animManager.getAnimID("Char1/particles/particle_jump"), 22,
                7}.setTiePosRules(TiePosRule::TIE_TO_GROUND)},
            {2, {}},
            }))
    ));

    sm.addState(std::unique_ptr<GenericState>(
        &(new PlayerState<false, false, false, InputComparatorTapUp, InputComparatorTapUp, true, InputComparatorIdle, InputComparatorIdle>(
            CharacterState::PREJUMP, {CharacterState::NONE, {CharacterState::IDLE, CharacterState::PRERUN, CharacterState::RUN, CharacterState::RUN_RECOVERY, CharacterState::LANDING_RECOVERY}}, m_animManager.getAnimID("Char1/prejump")))
        ->setGravity(Vector2<float>{0.0f, 0.0f})
        .setConvertVelocityOnSwitch(true, false)
        .setTransitionOnLostGround(CharacterState::FLOAT)
        .setMagnetLimit(TimelineProperty<unsigned int>(4))
        .setUpdateMovementData(
            TimelineProperty(Vector2<float>{1.0f, 1.0f}), // Vel mul
            TimelineProperty(Vector2<float>{0.0f, 0.0f}), // Dir vel mul
            TimelineProperty<Vector2<float>>(
                {
                    {0, {0.0f, 0.0f}},
                    {1, {0.0f, -4.5f}},
                }), // Raw vel
            TimelineProperty<Vector2<float>>(
                {
                    {0, {1.0f, 1.0f}},
                    {1, {0.5f, 1.0f}}
                }), // Inr mul
            TimelineProperty(Vector2<float>{0.0f, 0.0f}), // Dir inr mul
            TimelineProperty(Vector2<float>{0.0f, 0.0f})) // Raw inr
        .setDrag(TimelineProperty(Vector2<float>{0.0f, 0.0f}))
        .setAppliedInertiaMultiplier(TimelineProperty(Vector2<float>{0.0f, 0.0f}))
        .setUpdateSpeedLimitData(
            TimelineProperty<Vector2<float>>(),
            TimelineProperty(Vector2<float>{9999.9f, 4.0f}))
        .setHurtboxes({
            {
                HurtboxGroup(
                    {
                        {
                            {{{-6, -28}, {12, 28}}, TimelineProperty<bool>(true)}
                        }
                    }, HurtTrait::NORMAL
                )
            }
        })
        .setOutdatedTransition(CharacterState::FLOAT, 1)
        .setParticlesSingle(TimelineProperty<ParticleTemplate>({
            {0, {}},
            {1, ParticleTemplate{1, Vector2<float>{0.0f, 0.0f}, m_animManager.getAnimID("Char1/particles/particle_jump"), 22,
                7}.setTiePosRules(TiePosRule::TIE_TO_GROUND)},
            {2, {}},
            }))
    ));

    sm.addState(std::unique_ptr<GenericState>(
        &(new PlayerState<false, false, true, InputComparatorTapAttack, InputComparatorTapAttack, false, InputComparatorFail, InputComparatorFail>(
            CharacterState::ATTACK_1, {CharacterState::NONE, {CharacterState::IDLE, CharacterState::PRERUN, CharacterState::RUN, CharacterState::RUN_RECOVERY, CharacterState::LANDING_RECOVERY}}, m_animManager.getAnimID("Char1/attack1")))
        ->setLookaheadSpeedSensitivity(TimelineProperty<Vector2<float>>({
                    {0, {0.0f, 1.0f}},
                    {8, {1.0f, 1.0f}}
                }))
        .setCanFallThrough(TimelineProperty<bool>(false))
        .setGravity(Vector2<float>{0.0f, 0.0f})
        .setConvertVelocityOnSwitch(true, false)
        .setTransitionOnLostGround(CharacterState::FLOAT)
        .setMagnetLimit(TimelineProperty<unsigned int>({
                    {0, 10},
                    {10, 4}
                }))
        .setUpdateMovementData(
            TimelineProperty<Vector2<float>>( 
                {
                    {0, {1.0f, 1.0f}},
                    {5, {0.5f, 0.0f}}
                }), // Vel mul
            TimelineProperty<Vector2<float>>( 
                {
                    {0, {0.0f, 0.0f}},
                    {2, {2.0f, 0.0f}},
                    {5, {0.0f, 0.0f}}
                }),  // Dir vel mul
            TimelineProperty(Vector2<float>{0.0f, 0.0f}), // Raw vel
            TimelineProperty(Vector2<float>{1.0f, 1.0f}), // Inr mul
            TimelineProperty(Vector2<float>{0.0f, 0.0f}), // Dir inr mul
            TimelineProperty(Vector2<float>{0.0f, 0.0f})) // Raw inr
        .setMulInsidePushbox(TimelineProperty<std::optional<Vector2<float>>>({
            {2, Vector2<float>{0.2f, 1.0f}},
            {5, std::nullopt}
        }))
        .setTransitionVelocityMultiplier(TimelineProperty<Vector2<float>>({
            {0, {1.0f, 1.0f}},
            {2, {0.2f, 1.0f}},
            {10, {1.0f, 1.0f}}
        }))
        .setHurtboxes({
            {
                HurtboxGroup(
                    {
                        {
                            {{{-6, -28}, {12, 28}}, TimelineProperty<bool>(true)}
                        }
                    }, HurtTrait::NORMAL
                )
            }
        })
        .addHit(HitGeneration::hitPlayerLight())
        .setDrag(TimelineProperty<Vector2<float>>({
            {0, {0.05f, 0.05f}},
            {8, {0.3f, 0.3f}},
            }))
        .setRecoveryFrames(TimelineProperty<StateMarker>({
            {0, StateMarker{CharacterState::NONE, {}}},
            {16, StateMarker{CharacterState::NONE, {CharacterState::ATTACK_1_CHAIN}}},
            {25, StateMarker{CharacterState::NONE, {CharacterState::ATTACK_1_CHAIN, CharacterState::PREJUMP, CharacterState::PRERUN}}}
            }))
        .setParticlesSingle(TimelineProperty<ParticleTemplate>({
            {0, {}},
            {4, ParticleTemplate{1, Vector2<float>{3.0f, 3.0f}, m_animManager.getAnimID("Char1/particles/attack1_trace"), 10,
                3}
                .setTiePosRules(TiePosRule::TIE_TO_SOURCE)
                .setTieLifetimeRules(TieLifetimeRule::DESTROY_ON_STATE_LEAVE)
                .setNotDependOnGroundAngle()},
            {5, {}},
        }))
        .setOutdatedTransition(CharacterState::IDLE, 30)
    ));

    // TODO: grounded particles that only appear if there is a ground in front of you
    sm.addState(std::unique_ptr<GenericState>(
        new PlayerActionAttack1Chain(
            m_animManager.getAnimID("Char1/attack1_chain"), m_animManager.getAnimID("Char1/particles/attack1_chain_trace"))
    ));

    sm.addState(std::unique_ptr<GenericState>(
        &(new PlayerState<true, false, false, InputComparatorFail, InputComparatorFail, true, InputComparatorHoldLeft, InputComparatorHoldRight>(
            CharacterState::RUN, {CharacterState::NONE, {}}, m_animManager.getAnimID("Char1/run")))
        ->setGravity(Vector2<float>{0.0f, 0.0f})
        .setUpdateMovementData(
            TimelineProperty(Vector2<float>{1.0f, 1.0f}), // Vel mul
            TimelineProperty<Vector2<float>>( 
                {
                    {0, {0.4f, 0.0f}},
                    {5, {0.6f, 0.0f}},
                }),  // Dir vel mul
            TimelineProperty(Vector2<float>{0.0f, 0.0f}), // Raw vel
            TimelineProperty(Vector2<float>{1.0f, 1.0f}), // Inr mul
            TimelineProperty(Vector2<float>{0.0f, 0.0f}), // Dir inr mul
            TimelineProperty(Vector2<float>{0.0f, 0.0f})) // Raw inr
        .setUpdateSpeedLimitData(
            TimelineProperty(Vector2<float>{2.5f, 0.0f}),
            TimelineProperty(Vector2<float>{9999.9f, 0.0f}))
        .setTransitionOnLostGround(CharacterState::FLOAT)
        .setMagnetLimit(TimelineProperty<unsigned int>(4))
        .setHurtboxes({
            {
                HurtboxGroup(
                    {
                        {
                            {{{-6, -28}, {12, 28}}, TimelineProperty<bool>(true)}
                        }
                    }, HurtTrait::NORMAL
                )
            }
        })
        .setCanFallThrough(TimelineProperty<bool>(true))
        .setParticlesSingle(TimelineProperty<ParticleTemplate>({
            {0, {}},
            {1, ParticleTemplate{1, Vector2<float>{-10.0f, 0.0f}, m_animManager.getAnimID("Char1/particles/particle_run"), 26,
                7}.setTiePosRules(TiePosRule::TIE_TO_GROUND)},
            {2, {}},
            }))
        .setParticlesLoopable(TimelineProperty<ParticleTemplate>({
            {0, {}},
            {5, ParticleTemplate{1, Vector2<float>{-13.0f, 0.0f}, m_animManager.getAnimID("Char1/particles/particle_run_loop"), 21,
                7}.setTiePosRules(TiePosRule::TIE_TO_GROUND)},
            {6, {}},
            {30, ParticleTemplate{1, Vector2<float>{-13.0f, 0.0f}, m_animManager.getAnimID("Char1/particles/particle_run_loop"), 21,
                7}.setTiePosRules(TiePosRule::TIE_TO_GROUND)},
            {31, {}},
            }), 50)
    ));

    sm.addState(std::unique_ptr<GenericState>(
        &(new PlayerState<false, true, false, InputComparatorHoldLeft, InputComparatorHoldRight, true, InputComparatorHoldLeft, InputComparatorHoldRight>(
            CharacterState::PRERUN, {CharacterState::NONE, {CharacterState::IDLE, CharacterState::RUN, CharacterState::LANDING_RECOVERY}}, m_animManager.getAnimID("Char1/prerun")))
        ->setGravity(Vector2<float>{0.0f, 0.0f})
        .setConvertVelocityOnSwitch(true, false)
        .setTransitionOnLostGround(CharacterState::FLOAT)
        .setMagnetLimit(TimelineProperty<unsigned int>(4))
        .setUpdateMovementData(
            TimelineProperty(Vector2<float>{1.0f, 1.0f}), // Vel mul
            TimelineProperty<Vector2<float>>( 
                {
                    {0, {0.0f, 0.0f}},
                    {3, {0.3f, 0.0f}},
                }),  // Dir vel mul
            TimelineProperty(Vector2<float>{0.0f, 0.0f}), // Raw vel
            TimelineProperty(Vector2<float>{1.0f, 1.0f}), // Inr mul
            TimelineProperty(Vector2<float>{0.0f, 0.0f}), // Dir inr mul
            TimelineProperty(Vector2<float>{0.0f, 0.0f})) // Raw inr
        .setUpdateSpeedLimitData(
            TimelineProperty(Vector2<float>{2.5f, 0.0f}),
            TimelineProperty(Vector2<float>{9999.9f, 0.0f}))
        .setHurtboxes({
            {
                HurtboxGroup(
                    {
                        {
                            {{{-6, -28}, {12, 28}}, TimelineProperty<bool>(true)}
                        }
                    }, HurtTrait::NORMAL
                )
            }
        })
        .setOutdatedTransition(CharacterState::RUN, 5)
    ));

    sm.addState(std::unique_ptr<GenericState>(
        &(new PlayerState<true, false, false, InputComparatorIdle, InputComparatorIdle, false, InputComparatorIdle, InputComparatorIdle>(
            CharacterState::RUN_RECOVERY, {CharacterState::NONE, {CharacterState::PRERUN, CharacterState::RUN}}, m_animManager.getAnimID("Char1/run_recovery")))
        ->setGravity(Vector2<float>{0.0f, 0.0f})
        .setDrag(Vector2<float>{0.5f, 0.0f})
        .setConvertVelocityOnSwitch(true, false)
        .setTransitionOnLostGround(CharacterState::FLOAT)
        .setMagnetLimit(TimelineProperty<unsigned int>(4))
        .setUpdateMovementData(
            TimelineProperty(Vector2<float>{1.0f, 1.0f}), // Vel mul
            TimelineProperty(Vector2<float>{0.0f, 0.0f}),  // Dir vel mul
            TimelineProperty(Vector2<float>{0.0f, 0.0f}), // Raw vel
            TimelineProperty(Vector2<float>{1.0f, 1.0f}), // Inr mul
            TimelineProperty(Vector2<float>{0.0f, 0.0f}), // Dir inr mul
            TimelineProperty(Vector2<float>{0.0f, 0.0f})) // Raw inr
        .setUpdateSpeedLimitData(
            TimelineProperty(Vector2<float>{2.5f, 0.0f}),
            TimelineProperty(Vector2<float>{9999.9f, 0.0f}))
        .setHurtboxes({
            {
                HurtboxGroup(
                    {
                        {
                            {{{-6, -28}, {12, 28}}, TimelineProperty<bool>(true)}
                        }
                    }, HurtTrait::NORMAL
                )
            }
        })
        .setOutdatedTransition(CharacterState::IDLE, 9)
    ));

    sm.addState(std::unique_ptr<GenericState>(
        &(new PlayerState<false, false, false, InputComparatorIdle, InputComparatorIdle, false, InputComparatorIdle, InputComparatorIdle>(
            CharacterState::IDLE, {CharacterState::NONE, {}}, m_animManager.getAnimID("Char1/idle")))
        ->setGravity(Vector2<float>{0.0f, 0.0f})
        .setDrag(TimelineProperty<Vector2<float>>({{0, Vector2{0.1f, 0.1f}}, {3, Vector2{0.5f, 0.5f}}}))
        .setConvertVelocityOnSwitch(true, false)
        .setTransitionOnLostGround(CharacterState::FLOAT)
        .setMagnetLimit(TimelineProperty<unsigned int>(4))
        .setHurtboxes({
            {
                HurtboxGroup(
                    {
                        {
                            {{{-6, -28}, {12, 28}}, TimelineProperty<bool>(true)}
                        }
                    }, HurtTrait::NORMAL
                )
            }
        })
        .setCanFallThrough(TimelineProperty<bool>(true))
        .setUpdateSpeedLimitData(
            TimelineProperty(Vector2<float>{9999.9f, 0.0f}),
            TimelineProperty(Vector2<float>{9999.9f, 0.0f}))
    ));

    sm.addState(std::unique_ptr<GenericState>(
        &(new PlayerState<true, false, false, InputComparatorFail, InputComparatorFail, false, InputComparatorIdle, InputComparatorIdle>(
            CharacterState::LANDING_RECOVERY, {CharacterState::NONE, {}}, m_animManager.getAnimID("Char1/landing_recovery")))
        ->setGravity(Vector2<float>{0.0f, 0.0f})
        .setConvertVelocityOnSwitch(true, false)
        .setTransitionOnLostGround(CharacterState::FLOAT)
        .setMagnetLimit(TimelineProperty<unsigned int>(4))
        .setHurtboxes({
            {
                HurtboxGroup(
                    {
                        {
                            {{{-6, -28}, {12, 28}}, TimelineProperty<bool>(true)}
                        }
                    }, HurtTrait::NORMAL
                )
            }
        })
        .setOutdatedTransition(CharacterState::IDLE, 14)
        .setParticlesSingle(TimelineProperty<ParticleTemplate>({
            {0, ParticleTemplate{1, Vector2<float>{0.0f, 0.0f}, m_animManager.getAnimID("Char1/particles/particle_land"), 36,
                7}.setTiePosRules(TiePosRule::TIE_TO_GROUND)},
            {1, {}},
            }))
    ));

    sm.addState(std::unique_ptr<GenericState>(
        &(new PlayerState<true, false, false, InputComparatorFail, InputComparatorFail, false, InputComparatorIdle, InputComparatorIdle>(
            CharacterState::HARD_LANDING_RECOVERY, {CharacterState::NONE, {}}, m_animManager.getAnimID("Char1/landing_recovery")))
        ->setGravity(Vector2<float>{0.0f, 0.0f})
        .setConvertVelocityOnSwitch(true, false)
        .setTransitionOnLostGround(CharacterState::FLOAT)
        .setMagnetLimit(TimelineProperty<unsigned int>(4))
        .setHurtboxes({
            {
                HurtboxGroup(
                    {
                        {
                            {{{-6, -28}, {12, 28}}, TimelineProperty<bool>(true)}
                        }
                    }, HurtTrait::NORMAL
                )
            }
        })
        .setOutdatedTransition(CharacterState::IDLE, 14)
        .setParticlesSingle(TimelineProperty<ParticleTemplate>({
            {0, ParticleTemplate{1, Vector2<float>{0.0f, 0.0f}, m_animManager.getAnimID("Char1/particles/particle_land"), 36,
                7}.setTiePosRules(TiePosRule::TIE_TO_GROUND)},
            {1, {}},
            }))
    ));

    sm.addState(std::unique_ptr<GenericState>(
        &(new PlayerState<false, false, true, InputComparatorTapAttack, InputComparatorTapAttack, false, InputComparatorFail, InputComparatorFail>(
            CharacterState::AIR_ATTACK, {CharacterState::NONE, {CharacterState::FLOAT}}, m_animManager.getAnimID("Char1/air_attack")))
        ->allowAirDrift()
        .setLookaheadSpeedSensitivity(TimelineProperty<Vector2<float>>({
                    {0, {0.1f, 1.0f}},
                    {5, {0.2f, 1.0f}},
                    {10, {0.3f, 1.0f}},
                    {20, {1.0f, 1.0f}}
                }))
        .setCanFallThrough(TimelineProperty<bool>(false))
        .setGravity(Vector2<float>{0.0f, 0.5f})
        .setConvertVelocityOnSwitch(false, true)
        .setMagnetLimit(TimelineProperty<unsigned int>(0))
        .setNoLanding(TimelineProperty<bool>(false))
        .setHurtboxes({
            {
                HurtboxGroup(
                    {
                        {
                            {{{-6, -28}, {12, 28}}, TimelineProperty<bool>(true)}
                        }
                    }, HurtTrait::NORMAL
                )
            }
        })
        .addTransitionOnTouchedGround(0, CharacterState::HARD_LANDING_RECOVERY)
        .addTransitionOnTouchedGround(26, CharacterState::LANDING_RECOVERY)
        .addHit(HitGeneration::hitPlayerAirAttack())
        .setDrag(TimelineProperty(Vector2<float>{0.0f, 0.0f}))
        .setParticlesSingle(TimelineProperty<ParticleTemplate>({
            {0, {}},
            {7, ParticleTemplate{1, Vector2<float>{4.0f, 42.0f}, m_animManager.getAnimID("Char1/particles/air_attack_trace"), 14,
                3}
                .setTiePosRules(TiePosRule::TIE_TO_SOURCE)
                .setTieLifetimeRules(TieLifetimeRule::DESTROY_ON_STATE_LEAVE)
                .setNotDependOnGroundAngle()},
            {8, {}},
        }))
        .setOutdatedTransition(CharacterState::FLOAT, 35)
    ));

    sm.addState(std::unique_ptr<GenericState>(
        &(new PlayerActionFloat(
            m_animManager.getAnimID("Char1/float"), {CharacterState::NONE, {}}))
        ->allowAirDrift()
        .setLookaheadSpeedSensitivity(TimelineProperty<Vector2<float>>({
                    {0, {0.5f, 0.5f}},
                    {6, {1.0f, 1.0f}}
                }))
        .setGravity(Vector2<float>{0.0f, 0.5f})
        .setDrag(TimelineProperty(Vector2<float>{0.0f, 0.0f}))
        .setNoLanding(TimelineProperty<bool>({{0, true}, {4, false}}))
        .setHurtboxes({
            {
                HurtboxGroup(
                    {
                        {
                            {{{-6, -28}, {12, 28}}, TimelineProperty<bool>(true)}
                        }
                    }, HurtTrait::NORMAL
                )
            }
        })
        .setConvertVelocityOnSwitch(false, true)
    ));

    sm.setInitialState(CharacterState::FLOAT);
    smreset.m_defaultStates = {static_cast<CharState>(CharacterState::FLOAT)}; // TODO: allow any type
}

void PlayerSystem::update()
{
}
