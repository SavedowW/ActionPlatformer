#include "PlayerSystem.h"
#include "Hit.h"
#include "SM/Builder.hpp"  // IWYU pragma: keep
#include "SM/StateProperties.hpp"  // IWYU pragma: keep
#include "ResetHandlers.h"
#include "Core/Application.h"

PlayerSystem::PlayerSystem(entt::registry &reg_) :
    m_reg(reg_),
    m_animManager(Application::instance().m_animationManager)
{
    /*
        TODO: particles, hurtboxes, hitboxes
    */

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
            PlayerState::FLOAT,

            SM::CallBatch(PlayerStateProperties::Update::TestFallthrough{},
                PlayerStateProperties::Update::AirDrift{}),

            PlayerMake::SequentialConditions{}
                .addCondition(PlayerStateTransitions::sinceFrame(1, PlayerStateTransitions::OnGrounded{PlayerState::LANDING_RECOVERY}))
                .done(),

            [](const PlayerView&, const SM::TransitionData<PlayerState>&) {
            },

            PlayerMake::RulePipe{}
                .setDefaultPipe(PlayerStateProperties::Pipe::TestFallthrough{},
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


    phys.m_pushbox = {.m_topLeft=Vector2{-7.0f, -32.0f}, .m_size=Vector2{14.0f, 32.0f}};
    phys.m_gravity = {0.0f, 0.0f};


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
