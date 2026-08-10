#include "PlayerSystem.h"
#include "Hit.h"
#include "ParticleSystem.h"
#include "PlayableCharacter.h"
#include "SM/Builder.hpp"  // IWYU pragma: keep
#include "SM/StateProperties.hpp"  // IWYU pragma: keep
#include "ResetHandlers.h"
#include "Core/Application.h"

PlayerSystem::PlayerSystem(entt::registry &reg_, ParticleSystem &parSys_, Camera &cam_) :
    m_reg(reg_),
    m_parSys{parSys_},
    m_cam{cam_},
    m_animManager(Application::instance().m_animationManager)
{
    /*
        TODO: particles, hurtboxes, hitboxes
    */
    m_statemachine.addState(
        PlayerMake::state(
            PlayerState::ATTACK_1_CHAIN,

            SM::CallBatch(
                PlayerStateProperties::Update::EmitParticles{m_parSys, 
                    {
                        {
                            11, ParticleEmissionRuleset{
                                ParticleRecipe{m_animManager.getAnimID("Char1/particles/attack1_chain_trace"), 10, 3}
                                    .tiePos(TiePosRule::TIE_TO_EMITTER)
                            }.destroyOnStateChange()
                        }
                    }
                },
                PlayerStateProperties::Update::LookaheadSpeedSensitivity{TimelineProperty<Vector2<float>>( 
                    {
                        {0, {0.f, 0.f}},
                        {15, {1.f, 1.f}},
                        {40, {0.f, 0.f}},
                        {45, {1.f, 1.f}},
                    })},
                PlayerStateProperties::Update::MagnetLimit{TimelineProperty<unsigned int>( 
                    {
                        {0, 16},
                        {15, 4},
                    })},
                PlayerStateProperties::Update::MultiplyVelocity{TimelineProperty<Vector2<float>>( 
                    {
                        {0, {1.f, 1.f}},
                        {12, {.1f, 1.f}},
                        {30, {1.f, 1.f}},
                        {44, {0.f, 1.f}}
                    })},
                PlayerStateProperties::Update::AddOrientedVelocity{TimelineProperty<Vector2<float>>( 
                    {
                        {0, {0.f, 0.f}},
                        {9, {5.f, 0.f}},
                        {12, {0.f, 0.f}},
                        {40, {-1.5f, 0.f}},
                        {41, {0.f, 0.f}},
                    })},
                PlayerStateProperties::Update::SetDrag{TimelineProperty<Vector2<float>>( 
                    {
                        {0, {.05f, .05f}},
                        {8, {.3f, .3f}},
                    })},
                PlayerStateProperties::Update::CamShake{m_cam, {
                    {11, {.xAmp=15, .yAmp=15, .period=14}}
                }, GroundedCheck::ONLY_GROUNDED}
            ),

            PlayerMake::SequentialConditions{}
                .addCondition(std::make_unique<PlayerStateTransitions::OnGrounded>(PlayerState::FLOAT, false))
                .addCondition(std::make_unique<PlayerStateTransitions::OnTimer>(PlayerState::IDLE, 46))
                .done(),

            PlayerMake::RulePipe{}
                .setDefaultPipe(PlayerStateProperties::Pipe::MultiplyVelocity{{0.2f, 1.f}},
                                PlayerStateProperties::Pipe::DestroyParticlesOnLeave{m_reg})
                .done(),

            PlayerMake::RulePipe{}
                .setDefaultPipe(PlayerStateProperties::Pipe::Realign{},
                                PlayerStateProperties::Pipe::SetAnimation{m_animManager.getAnimID("Char1/attack1_chain")},
                                PlayerStateProperties::Pipe::SetLookaheadSpeedSensitivity{{0.f, 0.f}},
                                PlayerStateProperties::Pipe::SetGravity{{0.f, 0.f}},
                                PlayerStateProperties::Pipe::ConvertToInertia{true, false},
                                PlayerStateProperties::Pipe::SetMagnetLimit{16},
                                PlayerStateProperties::Pipe::SetDrag{{0.05f, 0.05f}})
                                
                .done()
    ));

    m_statemachine.addState(
        PlayerMake::state(
            PlayerState::ATTACK_1,

            SM::CallBatch(
                PlayerStateProperties::Update::EmitParticles{m_parSys, 
                    {
                        {
                            4, ParticleEmissionRuleset{
                                ParticleRecipe{m_animManager.getAnimID("Char1/particles/attack1_trace"), 10, 3}
                                    .tiePos(TiePosRule::TIE_TO_EMITTER)
                            }.destroyOnStateChange()
                        }
                    }
                },
                PlayerStateProperties::Update::MagnetLimit{TimelineProperty<unsigned int>( 
                    {
                        {0, 10},
                        {10, 4},
                    })},
                PlayerStateProperties::Update::SetDrag{TimelineProperty<Vector2<float>>( 
                    {
                        {0, {0.05f, 0.05f}},
                        {8, {0.3f, 0.3f}},
                    })},
                PlayerStateProperties::Update::LookaheadSpeedSensitivity{TimelineProperty<Vector2<float>>( 
                    {
                        {0, {0.f, 1.f}},
                        {8, {1.f, 1.f}},
                    })},
                PlayerStateProperties::Update::MultiplyVelocity{TimelineProperty<Vector2<float>>( 
                    {
                        {0, {1.0f, 1.0f}},
                        {5, {0.5f, 0.0f}}
                    })},
                PlayerStateProperties::Update::AddOrientedVelocity{TimelineProperty<Vector2<float>>( 
                    {
                        {0, {0.f, 0.f}},
                        {2, {2.f, 0.f}},
                        {5, {0.f, 0.f}}
                    })}
            ),

            PlayerMake::SequentialConditions{}
                .addCondition(PlayerStateTransitions::sinceFrame(16, PlayerStateTransitions::InputTest{PlayerState::ATTACK_1_CHAIN, InputMotions::BUFFERED_ORIENTED_ATTACK, Flag(OrientationOptions::LEFT) | OrientationOptions::RIGHT, 10}))
                .addCondition(PlayerStateTransitions::sinceFrame(25, PlayerStateTransitions::InputTest{PlayerState::PREJUMP_FORWARD, InputMotions::BUFFER_UP_FORWARD, Flag(OrientationOptions::LEFT) | OrientationOptions::RIGHT}))
                .addCondition(PlayerStateTransitions::sinceFrame(25, PlayerStateTransitions::InputTest{PlayerState::PREJUMP, InputMotions::BUFFER_UP, Flag(OrientationOptions::LEFT) | OrientationOptions::RIGHT}))
                .addCondition(PlayerStateTransitions::sinceFrame(25, PlayerStateTransitions::InputTest{PlayerState::PRERUN, InputMotions::HOLD_HORDIR, Flag(OrientationOptions::LEFT) | OrientationOptions::RIGHT}))
                .addCondition(std::make_unique<PlayerStateTransitions::OnGrounded>(PlayerState::FLOAT, false))
                .addCondition(std::make_unique<PlayerStateTransitions::OnTimer>(PlayerState::IDLE, 30))
                .done(),

            PlayerMake::RulePipe{}
                .setDefaultPipe(PlayerStateProperties::Pipe::MultiplyVelocity{{0.2f, 1.f}},
                                PlayerStateProperties::Pipe::DestroyParticlesOnLeave{m_reg})
                .done(),

            PlayerMake::RulePipe{}
                .setDefaultPipe(PlayerStateProperties::Pipe::Realign{},
                                PlayerStateProperties::Pipe::SetInertiaApplicationMultiplier{{0.0f, 0.0f}},
                                PlayerStateProperties::Pipe::SetLookaheadSpeedSensitivity{{0.0f, 1.0f}},
                                PlayerStateProperties::Pipe::SetGravity{{0.0f, 0.0f}},
                                PlayerStateProperties::Pipe::SetDrag{{0.05f, 0.05f}},
                                PlayerStateProperties::Pipe::SetMagnetLimit{10},
                                PlayerStateProperties::Pipe::SetAnimation{m_animManager.getAnimID("Char1/attack1")},
                                PlayerStateProperties::Pipe::ConvertToInertia{true, false})
                .done()
    ));
    
    m_statemachine.addState(
        PlayerMake::state(
            PlayerState::WALL_CLING_PREJUMP,
            
            SM::CallBatch(),

            PlayerMake::SequentialConditions{}
                .addCondition(PlayerStateTransitions::sinceFrame(1, PlayerStateTransitions::OnGrounded{PlayerState::LANDING_RECOVERY}))
                .addCondition(std::make_unique<PlayerStateTransitions::OnTimer>(PlayerState::FLOAT, 3))
                .addCondition(std::make_unique<PlayerStateTransitions::WallClingLeaveTest>(PlayerState::FLOAT))
                .done(),
            
            PlayerMake::RulePipe{}
                .setDefaultPipe(PlayerStateProperties::Pipe::SetDemandWall{false},
                                PlayerStateProperties::Pipe::SetInertiaApplicationMultiplier{{1.0f, 1.0f}},
                                PlayerStateProperties::Pipe::LeaveWallPrejump{})
                .done(),
            
            PlayerMake::RulePipe{}
                .setDefaultPipe(PlayerStateProperties::Pipe::SetDemandWall{true},
                                PlayerStateProperties::Pipe::SetInertiaApplicationMultiplier{{0.0f, 0.0f}},
                                PlayerStateProperties::Pipe::SetAnimation{m_animManager.getAnimID("Char1/wall_prejump")},
                                PlayerStateProperties::Pipe::HaltSideDownwardMomentum{},
                                PlayerStateProperties::Pipe::SetGravity{{0.0f, 0.02f}},
                                PlayerStateProperties::Pipe::SetDrag{{1.0f, 0.5f}},
                                PlayerStateProperties::Pipe::ConvertToInertia{true, true})
                .done()
    ));

    m_statemachine.addState(
        PlayerMake::state(
            PlayerState::WALL_CLING,
            
            SM::CallBatch(
                PlayerStateProperties::Update::UpdateGravity{TimelineProperty<Vector2<float>>({
                    {0, {0.0f, 0.02f}}
                })},
                PlayerStateProperties::Update::SetDrag{TimelineProperty<Vector2<float>>({
                    {0, {1.0f, 0.2f}},
                    {1, {1.0f, 0.4f}}
                })}
            ),

            PlayerMake::SequentialConditions{}
                .addCondition(PlayerStateTransitions::sinceFrame(5, PlayerStateTransitions::OnGrounded{PlayerState::LANDING_RECOVERY}))
                .addCondition(std::make_unique<PlayerStateTransitions::InputTest>(PlayerState::WALL_CLING_PREJUMP, InputMotions::TAP_ANY_EXCEPT_BACKWARDS, OrientationOptions::SAME))
                .addCondition(std::make_unique<PlayerStateTransitions::WallClingLeaveTest>(PlayerState::FLOAT))
                .done(),
            
            PlayerMake::RulePipe{}
                .setDefaultPipe(PlayerStateProperties::Pipe::SetDemandWall{false})
                .done(),
            
            PlayerMake::RulePipe{}
                .setDefaultPipe(PlayerStateProperties::Pipe::SetDemandWall{true},
                                PlayerStateProperties::Pipe::SetAnimation{m_animManager.getAnimID("Char1/wall_cling")},
                                PlayerStateProperties::Pipe::Realign{},
                                PlayerStateProperties::Pipe::HaltSideDownwardMomentum{},
                                PlayerStateProperties::Pipe::SetGravity{{0.0f, 0.5f}},
                                PlayerStateProperties::Pipe::SetDrag{{1.0f, 0.2f}},
                                PlayerStateProperties::Pipe::ConvertToInertia{true, false})
                .done()
    ));

    m_statemachine.addState(
        PlayerMake::state(
            PlayerState::PREJUMP_FORWARD,

            SM::CallBatch(
                PlayerStateProperties::Update::AddOrientedVelocity{TimelineProperty<Vector2<float>>( 
                    {
                        {0, {0.0f, 0.0f}},
                        {1, {1.5f, -4.5f}},
                    })},
                PlayerStateProperties::Update::MultiplyInertia{TimelineProperty<Vector2<float>>( 
                    {
                        {0, {1.0f, 1.0f}},
                        {1, {0.75f, 1.0f}},
                    })},
                PlayerStateProperties::Update::HorizontalInertiaLimit{TimelineProperty{std::pair<float, float>{-4.f, 4.f}}}
            ),

            PlayerMake::SequentialConditions{}
                .addCondition(std::make_unique<PlayerStateTransitions::OnGrounded>(PlayerState::FLOAT, false))
                .addCondition(std::make_unique<PlayerStateTransitions::OnTimer>(PlayerState::FLOAT, 2))
                .done(),

            PlayerMake::RulePipe{}
                .setDefaultPipe(PlayerStateProperties::Pipe::SetInertiaApplicationMultiplier{{1.0f, 1.0f}})
                .done(),

            PlayerMake::RulePipe{}
                .setDefaultPipe(PlayerStateProperties::Pipe::Realign{},
                                PlayerStateProperties::Pipe::SetInertiaApplicationMultiplier{{0.0f, 0.0f}},
                                PlayerStateProperties::Pipe::SetGravity{{0.0f, 0.0f}},
                                PlayerStateProperties::Pipe::SetDrag{{0.0f, 0.0f}},
                                PlayerStateProperties::Pipe::SetMagnetLimit{4},
                                PlayerStateProperties::Pipe::SetAnimation{m_animManager.getAnimID("Char1/prejump")},
                                PlayerStateProperties::Pipe::ConvertToInertia{true, false})
                .done()
    ));

    m_statemachine.addState(
        PlayerMake::state(
            PlayerState::PREJUMP,

            SM::CallBatch(
                PlayerStateProperties::Update::AddAbsoluteVelocity{TimelineProperty<Vector2<float>>( 
                    {
                        {0, {0.0f, 0.0f}},
                        {1, {0.0f, -4.5f}},
                    })},
                PlayerStateProperties::Update::MultiplyInertia{TimelineProperty<Vector2<float>>( 
                    {
                        {0, {1.0f, 1.0f}},
                        {1, {0.75f, 1.0f}},
                    })}
            ),

            PlayerMake::SequentialConditions{}
                .addCondition(std::make_unique<PlayerStateTransitions::OnGrounded>(PlayerState::FLOAT, false))
                .addCondition(std::make_unique<PlayerStateTransitions::OnTimer>(PlayerState::FLOAT, 2))
                .done(),

            PlayerMake::RulePipe{}
                .setDefaultPipe(PlayerStateProperties::Pipe::SetInertiaApplicationMultiplier{{1.0f, 1.0f}})
                .done(),

            PlayerMake::RulePipe{}
                .setDefaultPipe(PlayerStateProperties::Pipe::SetInertiaApplicationMultiplier{{0.0f, 0.0f}},
                                PlayerStateProperties::Pipe::SetGravity{{0.0f, 0.0f}},
                                PlayerStateProperties::Pipe::SetDrag{{0.0f, 0.0f}},
                                PlayerStateProperties::Pipe::SetMagnetLimit{4},
                                PlayerStateProperties::Pipe::SetAnimation{m_animManager.getAnimID("Char1/prejump")},
                                PlayerStateProperties::Pipe::ConvertToInertia{true, false})
                .done()
    ));

    m_statemachine.addState(
        PlayerMake::state(
            PlayerState::RUN_RECOVERY,

            SM::CallBatch(
                PlayerStateProperties::Update::HorizontalVelocityLimit{TimelineProperty{std::pair<float, float>{-2.5f, 2.5f}}}
            ),

            PlayerMake::SequentialConditions{}
                .addCondition(std::make_unique<PlayerStateTransitions::OnGrounded>(PlayerState::FLOAT, false))
                .addCondition(std::make_unique<PlayerStateTransitions::InputTest>(PlayerState::ATTACK_1, InputMotions::BUFFERED_ORIENTED_ATTACK, Flag(OrientationOptions::LEFT) | OrientationOptions::RIGHT))
                .addCondition(std::make_unique<PlayerStateTransitions::InputTest>(PlayerState::PREJUMP_FORWARD, InputMotions::BUFFER_UP_FORWARD, Flag(OrientationOptions::LEFT) | OrientationOptions::RIGHT))
                .addCondition(std::make_unique<PlayerStateTransitions::InputTest>(PlayerState::PREJUMP, InputMotions::BUFFER_UP, Flag(OrientationOptions::LEFT) | OrientationOptions::RIGHT))
                .addCondition(std::make_unique<PlayerStateTransitions::InputTest>(PlayerState::PRERUN, InputMotions::HOLD_HORDIR, OrientationOptions::OPPOSITE))
                .addCondition(std::make_unique<PlayerStateTransitions::OnTimer>(PlayerState::IDLE, 9))
                .done(),

            PlayerMake::RulePipe{}
                .setDefaultPipe(PlayerStateProperties::Pipe::SetDrag{{0.0f, 0.0f}},
                                PlayerStateProperties::Pipe::SetMagnetLimit{0})
                .done(),

            PlayerMake::RulePipe{}
                .setDefaultPipe(PlayerStateProperties::Pipe::SetDrag{{0.5f, 0.0f}},
                                PlayerStateProperties::Pipe::SetMagnetLimit{4},
                                PlayerStateProperties::Pipe::SetGravity{{0.0f, 0.0f}},
                                PlayerStateProperties::Pipe::SetAnimation{m_animManager.getAnimID("Char1/run_recovery")},
                                PlayerStateProperties::Pipe::ConvertToInertia{true, false})
                .done()
    ));

    m_statemachine.addState(
        PlayerMake::state(
            PlayerState::RUN,

            SM::CallBatch(
                PlayerStateProperties::Update::AddOrientedVelocity{TimelineProperty<Vector2<float>>( 
                    {
                        {0, {0.4f, 0.0f}},
                        {4, {0.6f, 0.0f}},
                    })},
                PlayerStateProperties::Update::HorizontalVelocityLimit{TimelineProperty{std::pair<float, float>{-2.5f, 2.5f}}},
                PlayerStateProperties::Update::TestFallthrough{}),

            PlayerMake::SequentialConditions{}
                .addCondition(std::make_unique<PlayerStateTransitions::OnGrounded>(PlayerState::FLOAT, false))
                .addCondition(std::make_unique<PlayerStateTransitions::InputTest>(PlayerState::ATTACK_1, InputMotions::BUFFERED_ORIENTED_ATTACK, Flag(OrientationOptions::LEFT) | OrientationOptions::RIGHT))
                .addCondition(std::make_unique<PlayerStateTransitions::InputTest>(PlayerState::PREJUMP_FORWARD, InputMotions::BUFFER_UP_FORWARD, Flag(OrientationOptions::LEFT) | OrientationOptions::RIGHT))
                .addCondition(std::make_unique<PlayerStateTransitions::InputTest>(PlayerState::PREJUMP, InputMotions::BUFFER_UP, Flag(OrientationOptions::LEFT) | OrientationOptions::RIGHT))
                .addCondition(std::make_unique<PlayerStateTransitions::InputTest>(PlayerState::PRERUN, InputMotions::HOLD_HORDIR, OrientationOptions::OPPOSITE))
                .addCondition(std::make_unique<PlayerStateTransitions::InputTest>(PlayerState::RUN_RECOVERY, InputMotions::CHECK_NO_HORDIR, OrientationOptions::SAME))
                .done(),

            PlayerMake::RulePipe{}
                .setDefaultPipe(PlayerStateProperties::Pipe::SetDrag{{0.0f, 0.0f}},
                                PlayerStateProperties::Pipe::SetMagnetLimit{0})
                .done(),

            PlayerMake::RulePipe{}
                .setDefaultPipe(PlayerStateProperties::Pipe::SetMagnetLimit{4},
                                PlayerStateProperties::Pipe::SetDrag{{0.5f, 0.0f}},
                                PlayerStateProperties::Pipe::TestFallthrough{},
                                PlayerStateProperties::Pipe::AddOrientedVelocity{{0.4f, 0.0f}},
                                PlayerStateProperties::Pipe::SetGravity{{0.0f, 0.0f}},
                                PlayerStateProperties::Pipe::SetAnimation{m_animManager.getAnimID("Char1/run")})
                .done()
    ));

    m_statemachine.addState(
        PlayerMake::state(
            PlayerState::PRERUN,

            SM::CallBatch(
                PlayerStateProperties::Update::AddOrientedVelocity{TimelineProperty<Vector2<float>>(
                    {
                        {0, {0.0f, 0.0f}},
                        {1, {0.3f, 0.0f}},
                    })},
                PlayerStateProperties::Update::HorizontalVelocityLimit{TimelineProperty{std::pair<float, float>{-2.5f, 2.5f}}},
                PlayerStateProperties::Update::TestFallthrough{}),

            PlayerMake::SequentialConditions{}
                .addCondition(std::make_unique<PlayerStateTransitions::OnGrounded>(PlayerState::FLOAT, false))
                .addCondition(std::make_unique<PlayerStateTransitions::InputTest>(PlayerState::ATTACK_1, InputMotions::BUFFERED_ORIENTED_ATTACK, Flag(OrientationOptions::LEFT) | OrientationOptions::RIGHT))
                .addCondition(std::make_unique<PlayerStateTransitions::InputTest>(PlayerState::PREJUMP_FORWARD, InputMotions::BUFFER_UP_FORWARD, Flag(OrientationOptions::LEFT) | OrientationOptions::RIGHT))
                .addCondition(std::make_unique<PlayerStateTransitions::InputTest>(PlayerState::PREJUMP, InputMotions::BUFFER_UP, Flag(OrientationOptions::LEFT) | OrientationOptions::RIGHT))
                .addCondition(std::make_unique<PlayerStateTransitions::InputTest>(PlayerState::PRERUN, InputMotions::HOLD_HORDIR, OrientationOptions::OPPOSITE))
                .addCondition(std::make_unique<PlayerStateTransitions::OnTimer>(PlayerState::RUN, 5))
                .done(),

            PlayerMake::RulePipe{}
                .setDefaultPipe(PlayerStateProperties::Pipe::SetDrag{{0.0f, 0.0f}},
                                PlayerStateProperties::Pipe::SetMagnetLimit{0})
                .done(),

            PlayerMake::RulePipe{}
                .setDefaultPipe(PlayerStateProperties::Pipe::SetDrag{{0.5f, 0.0f}},
                                PlayerStateProperties::Pipe::TestFallthrough{},
                                PlayerStateProperties::Pipe::SetMagnetLimit{4},
                                PlayerStateProperties::Pipe::Realign{},
                                PlayerStateProperties::Pipe::SetGravity{{0.0f, 0.0f}},
                                PlayerStateProperties::Pipe::SetAnimation{m_animManager.getAnimID("Char1/prerun")},
                                PlayerStateProperties::Pipe::ConvertToInertia{true, false})
                .done()
    ));

    m_statemachine.addState(
        PlayerMake::state(
            PlayerState::IDLE,

            SM::CallBatch(
                PlayerStateProperties::Update::SetDrag{TimelineProperty<Vector2<float>>({
                        {0, {0.1f, 0.0f}},
                        {2, {0.5f, 0.0f}},
                })},
                PlayerStateProperties::Update::TestFallthrough{}
            ),

            PlayerMake::SequentialConditions{}
                .addCondition(std::make_unique<PlayerStateTransitions::OnGrounded>(PlayerState::FLOAT, false))
                .addCondition(std::make_unique<PlayerStateTransitions::InputTest>(PlayerState::ATTACK_1, InputMotions::BUFFERED_ORIENTED_ATTACK, Flag(OrientationOptions::LEFT) | OrientationOptions::RIGHT))
                .addCondition(std::make_unique<PlayerStateTransitions::InputTest>(PlayerState::PREJUMP_FORWARD, InputMotions::BUFFER_UP_FORWARD, Flag(OrientationOptions::LEFT) | OrientationOptions::RIGHT))
                .addCondition(std::make_unique<PlayerStateTransitions::InputTest>(PlayerState::PREJUMP, InputMotions::BUFFER_UP, Flag(OrientationOptions::LEFT) | OrientationOptions::RIGHT))
                .addCondition(std::make_unique<PlayerStateTransitions::InputTest>(PlayerState::PRERUN, InputMotions::HOLD_HORDIR, Flag(OrientationOptions::LEFT) | OrientationOptions::RIGHT))
                .done(),

            PlayerMake::RulePipe{}
                .setDefaultPipe(PlayerStateProperties::Pipe::SetDrag{{0.0f, 0.0f}},
                                PlayerStateProperties::Pipe::SetMagnetLimit{0})
                .done(),

            PlayerMake::RulePipe{}
                .setDefaultPipe(PlayerStateProperties::Pipe::SetDrag{{0.1f, 0.0f}},
                                PlayerStateProperties::Pipe::TestFallthrough{},
                                PlayerStateProperties::Pipe::SetMagnetLimit{4},
                                PlayerStateProperties::Pipe::SetGravity{{0.0f, 0.0f}},
                                PlayerStateProperties::Pipe::SetAnimation{m_animManager.getAnimID("Char1/idle")},
                                PlayerStateProperties::Pipe::ConvertToInertia{true, false})
                .done()
    ));

    m_statemachine.addState(
        PlayerMake::state(
            PlayerState::LANDING_RECOVERY,

            SM::CallBatch(PlayerStateProperties::Update::TestFallthrough{}),

            PlayerMake::SequentialConditions{}
                .addCondition(std::make_unique<PlayerStateTransitions::OnGrounded>(PlayerState::FLOAT, false))
                .addCondition(std::make_unique<PlayerStateTransitions::InputTest>(PlayerState::ATTACK_1, InputMotions::BUFFERED_ORIENTED_ATTACK, Flag(OrientationOptions::LEFT) | OrientationOptions::RIGHT))
                .addCondition(std::make_unique<PlayerStateTransitions::InputTest>(PlayerState::PREJUMP_FORWARD, InputMotions::BUFFER_UP_FORWARD, Flag(OrientationOptions::LEFT) | OrientationOptions::RIGHT))
                .addCondition(std::make_unique<PlayerStateTransitions::InputTest>(PlayerState::PREJUMP, InputMotions::BUFFER_UP, Flag(OrientationOptions::LEFT) | OrientationOptions::RIGHT))
                .addCondition(std::make_unique<PlayerStateTransitions::InputTest>(PlayerState::PRERUN, InputMotions::HOLD_HORDIR, Flag(OrientationOptions::LEFT) | OrientationOptions::RIGHT))
                .addCondition(std::make_unique<PlayerStateTransitions::OnTimer>(PlayerState::IDLE, 14))
                .done(),

            PlayerMake::RulePipe{}
                .setDefaultPipe(PlayerStateProperties::Pipe::SetMagnetLimit{0})
                .done(),

            PlayerMake::RulePipe{}
                .setDefaultPipe(PlayerStateProperties::Pipe::SetDrag{{1.0f, 0.0f}},
                                PlayerStateProperties::Pipe::TestFallthrough{},
                                PlayerStateProperties::Pipe::SetMagnetLimit{4},
                                PlayerStateProperties::Pipe::SetGravity{{0.0f, 0.0f}},
                                PlayerStateProperties::Pipe::SetAnimation{m_animManager.getAnimID("Char1/landing_recovery")},
                                PlayerStateProperties::Pipe::ConvertToInertia{true, false})
                .done()
    ));

    m_statemachine.addState(
        PlayerMake::state(
            PlayerState::HARD_LANDING_RECOVERY,

            SM::CallBatch(),

            PlayerMake::SequentialConditions{}
                .addCondition(std::make_unique<PlayerStateTransitions::OnGrounded>(PlayerState::FLOAT, false))
                .addCondition(std::make_unique<PlayerStateTransitions::OnTimer>(PlayerState::IDLE, 14))
                .done(),

            PlayerMake::RulePipe{}
                .setDefaultPipe(PlayerStateProperties::Pipe::SetMagnetLimit{0})
                .done(),

            PlayerMake::RulePipe{}
                .setDefaultPipe(PlayerStateProperties::Pipe::CamShake{m_cam, {.xAmp=0, .yAmp=20, .period=10}},
                                PlayerStateProperties::Pipe::SetDrag{{1.0f, 0.0f}},
                                PlayerStateProperties::Pipe::TestFallthrough{},
                                PlayerStateProperties::Pipe::SetMagnetLimit{4},
                                PlayerStateProperties::Pipe::SetGravity{{0.0f, 0.0f}},
                                PlayerStateProperties::Pipe::SetAnimation{m_animManager.getAnimID("Char1/landing_recovery")},
                                PlayerStateProperties::Pipe::ConvertToInertia{true, false})
                .done()
    ));

    m_statemachine.addState(
        PlayerMake::state(
            PlayerState::FLOAT,

            SM::CallBatch(PlayerStateProperties::Update::TestFallthrough{},
                PlayerStateProperties::Update::AirDrift{},
                PlayerStateProperties::Update::Realign{TimelineProperty{true}}),

            PlayerMake::SequentialConditions{}
                .addCondition(PlayerStateTransitions::sinceFrame(1, PlayerStateTransitions::OnGroundedBySpeed{PlayerState::HARD_LANDING_RECOVERY, true, 20.f}))
                .addCondition(PlayerStateTransitions::sinceFrame(1, PlayerStateTransitions::OnGrounded{PlayerState::LANDING_RECOVERY}))
                .addCondition(PlayerStateTransitions::sinceFrame(5, PlayerStateTransitions::WallClingEnterTest{PlayerState::WALL_CLING}))
                .done(),

            PlayerMake::RulePipe{}
                .setDefaultPipe(PlayerStateProperties::Pipe::SetDemandWall{false})
                .done(),

            PlayerMake::RulePipe{}
                .setDefaultPipe(PlayerStateProperties::Pipe::TestFallthrough{},
                                PlayerStateProperties::Pipe::SetDemandWall{true},
                                PlayerStateProperties::Pipe::SetGravity{{0.0f, 0.5f}},
                                PlayerStateProperties::Pipe::SetAnimation{m_animManager.getAnimID("Char1/float")},
                                PlayerStateProperties::Pipe::ConvertToInertia{false, true})
                .done()
    ));
}

void PlayerSystem::createPlayer()
{
    if (m_playerId != entt::null)
        m_reg.destroy(m_playerId);

    m_playerId = m_reg.create();
    m_reg.emplace<ComponentName>(m_playerId, "Player");

    auto &trans = m_reg.emplace<ComponentTransform>(m_playerId);
    m_reg.emplace<ComponentReset<ComponentTransform>>(m_playerId, trans.m_pos, trans.m_orientation);
    
    m_reg.emplace<ComponentChildParticles>(m_playerId);

    auto &phys = m_reg.emplace<ComponentPhysical>(m_playerId);
    m_reg.emplace<ComponentResetStatic<ComponentPhysical>>(m_playerId);

    m_reg.emplace<ComponentObstacleFallthrough>(m_playerId);
    
    auto &animrnd = m_reg.emplace<ComponentAnimationRenderable>(m_playerId);
    m_reg.emplace<ComponentResetStatic<ComponentAnimationRenderable>>(m_playerId);

    m_reg.emplace<RenderLayer>(m_playerId, 6);
    m_reg.emplace<InputResolver>(m_playerId);

    m_reg.emplace<ComponentDynamicCameraTarget>(m_playerId);
    m_reg.emplace<ComponentResetStatic<ComponentDynamicCameraTarget>>(m_playerId);

    m_reg.emplace<SM::StatePossessor<PlayerState>>(m_playerId, PlayerState::FLOAT);
    // m_reg.emplace<StateMachine>(m_playerId); TODO:
    // m_reg.emplace<ComponentReset<StateMachine>>(m_playerId);

    m_reg.emplace<WorldPosition>(m_playerId);
    m_reg.emplace<BattleActor>(m_playerId, BattleTeams::PLAYER);
    m_reg.emplace<HUDPoint>(m_playerId, HUDPosRule::REL_TRANSFORM, Vector2{0, -16}, 16);
    m_reg.emplace<HealthOwner>(m_playerId, 3);
    m_reg.emplace<HealthRendererCommonWRT>(m_playerId, 3, Vector2{0.0f, -28.0f});

    const auto spawnView = m_reg.view<ComponentSpawnLocation>().each();
    if (spawnView.cbegin() != spawnView.cend())
        trans.m_pos = std::get<ComponentSpawnLocation&>(*spawnView.cbegin()).location;

    animrnd.m_drawOutline = true;
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


    phys.pushbox = {.m_topLeft=Vector2{-7.0f, -32.0f}, .m_size=Vector2{14.0f, 32.0f}};
    phys.gravity = {0.0f, 0.0f};


    m_statemachine.init(m_reg, m_playerId);

    //sm.setInitialState(PlayerState::FLOAT);
}

void PlayerSystem::update()
{
    m_statemachine.update(m_reg);
}

entt::entity PlayerSystem::getPlayerId() const noexcept
{
    return m_playerId;
}
