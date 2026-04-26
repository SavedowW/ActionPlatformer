#include "PlayerSystem.h"
#include "SM/Builder.hpp"
#include "SM/StateProperties.hpp"
#include "ResetHandlers.h"
#include "Core/Application.h"

PlayerSystem::PlayerSystem(entt::registry &reg_) :
    m_reg(reg_),
    m_animManager(Application::instance().m_animationManager)
{
    
}

void PlayerSystem::setup(entt::entity playerId_)
{
    auto [trans, phys, inp, animrnd, state, transreset/*, smreset*/] = m_reg.get<ComponentTransform, ComponentPhysical, InputResolver, ComponentAnimationRenderable, SM::StatePossessor<PlayerState>,
        ComponentReset<ComponentTransform>/*, ComponentReset<SM::StatePossessor<PlayerState>>*/>(playerId_);

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
    phys.m_gravity = {0.0f, 0.0f};


    // TODO: Configure SM

    /*
    sm.addState<PlayerState<false, false, false, InputComparatorIdle, InputComparatorIdle, false, InputComparatorIdle, InputComparatorIdle>>(
            CharacterState::IDLE, StateMarker{}, m_animManager.getAnimID("Char1/idle"))
        .setGravity(Vector2<float>{0.0f, 0.0f})
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
            TimelineProperty(Vector2<float>{9999.9f, 0.0f}));
    */


    /*
        TODO: particles, hurtboxes, hitboxes
    */

    m_statemachine.addState(
        PlayerMake::state(
            PlayerState::RUN_RECOVERY,

            SM::CallBatch(
                PlayerStateProperties::Update::HorizontalVelocityLimit{TimelineProperty{std::pair<float, float>{-2.5f, 2.5f}}}
            ),

            PlayerMake::SequentialConditions{}
                .addCondition(std::make_unique<PlayerStateTransitions::OnPhysEvent>(PlayerState::FLOAT, PhysicalEvents::Events::GROUNDED, false))
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
                .addCondition(std::make_unique<PlayerStateTransitions::OnPhysEvent>(PlayerState::FLOAT, PhysicalEvents::Events::GROUNDED, false))
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
                .addCondition(std::make_unique<PlayerStateTransitions::OnPhysEvent>(PlayerState::FLOAT, PhysicalEvents::Events::GROUNDED, false))
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
                .addCondition(std::make_unique<PlayerStateTransitions::OnPhysEvent>(PlayerState::FLOAT, PhysicalEvents::Events::GROUNDED, false))
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

            SM::CallBatch(),

            PlayerMake::SequentialConditions{}
                .addCondition(std::make_unique<PlayerStateTransitions::OnPhysEvent>(PlayerState::FLOAT, PhysicalEvents::Events::GROUNDED, false))
                .addCondition(std::make_unique<PlayerStateTransitions::InputTest>(PlayerState::PRERUN, InputMotions::HOLD_HORDIR, Flag(OrientationOptions::LEFT) | OrientationOptions::RIGHT))
                .addCondition(std::make_unique<PlayerStateTransitions::OnTimer>(PlayerState::IDLE, 14))
                .done(),

            PlayerMake::RulePipe{}
                .setDefaultPipe(PlayerStateProperties::Pipe::SetInertiaApplicationMultiplier{{1.0f, 1.0f}},
                                PlayerStateProperties::Pipe::SetMagnetLimit{0})
                .done(),

            PlayerMake::RulePipe{}
                .setDefaultPipe(PlayerStateProperties::Pipe::SetInertiaApplicationMultiplier{{0.0f, 0.0f}},
                                PlayerStateProperties::Pipe::SetMagnetLimit{4},
                                PlayerStateProperties::Pipe::SetGravity{{0.0f, 0.0f}},
                                PlayerStateProperties::Pipe::SetAnimation{m_animManager.getAnimID("Char1/landing_recovery")},
                                PlayerStateProperties::Pipe::ConvertToInertia{true, false})
                .done()
    ));

    m_statemachine.addState(
        PlayerMake::state(
            PlayerState::FLOAT,

            SM::CallBatch(),

            PlayerMake::SequentialConditions{}
                .addCondition(std::make_unique<PlayerStateTransitions::OnPhysEvent>(PlayerState::LANDING_RECOVERY, PhysicalEvents::Events::GROUNDED))
                .done(),

            [](const PlayerView&, const SM::TransitionData<PlayerState>&) {
            },

            PlayerMake::RulePipe{}
                .setDefaultPipe(PlayerStateProperties::Pipe::SetGravity{{0.0f, 0.5f}},
                                PlayerStateProperties::Pipe::SetAnimation{m_animManager.getAnimID("Char1/float")},
                                PlayerStateProperties::Pipe::ConvertToInertia{false, true})
                .done()
    ));

    m_statemachine.init(m_reg, playerId_);

    //sm.setInitialState(PlayerState::FLOAT);
    //smreset.m_defaultStates = {static_cast<CharState>(PlayerState::FLOAT)}; // TODO: allow any type
}

void PlayerSystem::update()
{
    m_statemachine.update(m_reg);
}
