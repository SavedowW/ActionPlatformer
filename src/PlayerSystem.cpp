#include "PlayerSystem.h"
#include "SM/Builder.hpp"
#include "SM/PhysicalState.h"
#include "SM/PhysicalState.hpp"
#include "Core/InputResolver.h"
#include "PlayableCharacter.h"
#include "SM/StateMachine.hpp"
#include "ResetHandlers.h"
#include "Core/Application.h"
#include "SM/TransitionCheck.h"

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

    m_statemachine.addState(
        SM::Make<PlayerState, PlayerView>::state(
            PlayerState::IDLE,

            SM::CallBatch(
                SM::Updaters::Notify<PlayerState, PlayerView>()),

            [](const PlayerView&) {
                std::cout << "IDLE asked" << std::endl;
                return SM::TransitionData<PlayerState>{PlayerState::IDLE, PlayerState::IDLE, ORIENTATION::UNSPECIFIED};
            },

            [](const PlayerView&, const SM::TransitionData<PlayerState>&) {
                std::cout << "IDLE from" << std::endl;
            },
            PlayerMake::RulePipe{}
                .setDefaultPipe(SM::Transitions::Rules::In::Notify<PlayerState, PlayerView>{},
                                SM::Transitions::Rules::In::SetGravity<PlayerState, PlayerView>{{0.0f, 0.0f}},
                                SM::Transitions::Rules::In::SetAnimation<PlayerState, PlayerView>{m_animManager.getAnimID("Char1/idle")})
                .done()
    ));

    m_statemachine.addState(
        SM::Make<PlayerState, PlayerView>::state(
            PlayerState::LANDING_RECOVERY,

            SM::CallBatch(
                SM::Updaters::Notify<PlayerState, PlayerView>()),

            PlayerMake::SequentialConditions{}
                .addCondition(std::make_unique<SM::Transition::Checks::OnTimer<PlayerState, PlayerView>>(PlayerState::IDLE, 14))
                .done(),

            [](const PlayerView&, const SM::TransitionData<PlayerState>&) {
                std::cout << "LANDING_RECOVERY from" << std::endl;
            },
            PlayerMake::RulePipe{}
                .setDefaultPipe(SM::Transitions::Rules::In::Notify<PlayerState, PlayerView>{},
                                SM::Transitions::Rules::In::SetGravity<PlayerState, PlayerView>{{0.0f, 0.0f}},
                                SM::Transitions::Rules::In::SetAnimation<PlayerState, PlayerView>{m_animManager.getAnimID("Char1/landing_recovery")})
                .done()
    ));

    m_statemachine.addState(
        SM::Make<PlayerState, PlayerView>::state(
            PlayerState::FLOAT,

            SM::CallBatch(
                SM::Updaters::Notify<PlayerState, PlayerView>()),

            PlayerMake::SequentialConditions{}
                .addCondition(std::make_unique<SM::Transition::Checks::OnPhysEvent<PlayerState, PlayerView>>(PlayerState::LANDING_RECOVERY, PhysicalEvents::Events::TOUCHED_GROUND))
                .done(),

            [](const PlayerView&, const SM::TransitionData<PlayerState>&) {
                std::cout << "Float from" << std::endl;
            },
            PlayerMake::RulePipe{}
                .setDefaultPipe(SM::Transitions::Rules::In::Notify<PlayerState, PlayerView>{},
                                SM::Transitions::Rules::In::SetGravity<PlayerState, PlayerView>{{0.0f, 0.5f}},
                                SM::Transitions::Rules::In::SetAnimation<PlayerState, PlayerView>{m_animManager.getAnimID("Char1/float")})
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
