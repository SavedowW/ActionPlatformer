#pragma once
#include "Core/GameData.h"
#include "Core/CoreComponents.h"
#include "Core/InputResolver.h"
#include "Core/InputState.h"
#include "Core/Vector2.hpp"
#include "StateMachine.hpp"
#include "ParticleSystem.hpp"
#include "StateProperties.h"

template<typename StateIDT, typename ViewT>
void StateProperties<StateIDT, ViewT>::Update::Notify::operator()(const ViewT &view_) const
{
    const SM::StatePossessor<StateIDT> &owner = view_.template cget<SM::StatePossessor<StateIDT>>();
    std::cout << serialize(owner.stateId()) << ":" << owner.framesInState() << ": update" << std::endl;
}


template<typename StateIDT, typename ViewT>
StateProperties<StateIDT, ViewT>::Update::UpdateGravity::UpdateGravity(TimelineProperty<Vector2<float>> &&gravity_) :
    m_gravity{std::move(gravity_)}
{}

template<typename StateIDT, typename ViewT>
void StateProperties<StateIDT, ViewT>::Update::UpdateGravity::operator()(const ViewT &view_) const
{
    auto &physical = view_.template get<ComponentPhysical>();
    const SM::StatePossessor<StateIDT> &owner = view_.template cget<SM::StatePossessor<StateIDT>>();
    physical.gravity = m_gravity[owner.framesInState()];
}


template<typename StateIDT, typename ViewT>
StateProperties<StateIDT, ViewT>::Update::MultiplyVelocity::MultiplyVelocity(TimelineProperty<Vector2<float>> &&multiplier_) :
    m_multiplier{std::move(multiplier_)}
{}

template<typename StateIDT, typename ViewT>
void StateProperties<StateIDT, ViewT>::Update::MultiplyVelocity::operator()(const ViewT &view_) const
{
    auto &physical = view_.template get<ComponentPhysical>();
    physical.velocity = physical.velocity.mulComponents(m_multiplier[view_.template cget<SM::StatePossessor<StateIDT>>().framesInState()]);
}

template<typename StateIDT, typename ViewT>
StateProperties<StateIDT, ViewT>::Update::AddOrientedVelocity::AddOrientedVelocity(TimelineProperty<Vector2<float>> &&velocity_) :
    m_velocity{std::move(velocity_)}
{}

template<typename StateIDT, typename ViewT>
void StateProperties<StateIDT, ViewT>::Update::AddOrientedVelocity::operator()(const ViewT &view_) const
{
    auto velocityChange = m_velocity[view_.template cget<SM::StatePossessor<StateIDT>>().framesInState()];
    if (view_.template get<ComponentTransform>().m_orientation == ORIENTATION::LEFT)
        velocityChange.x *= -1;
    view_.template get<ComponentPhysical>().velocity += velocityChange;
}

template<typename StateIDT, typename ViewT>
StateProperties<StateIDT, ViewT>::Update::AddAbsoluteVelocity::AddAbsoluteVelocity(TimelineProperty<Vector2<float>> &&velocity_) :
    m_velocity{std::move(velocity_)}
{}

template<typename StateIDT, typename ViewT>
void StateProperties<StateIDT, ViewT>::Update::AddAbsoluteVelocity::operator()(const ViewT &view_) const
{
    auto &physical = view_.template get<ComponentPhysical>();
    physical.velocity += m_velocity[view_.template cget<SM::StatePossessor<StateIDT>>().framesInState()];
}

template<typename StateIDT, typename ViewT>
StateProperties<StateIDT, ViewT>::Update::MultiplyInertia::MultiplyInertia(TimelineProperty<Vector2<float>> &&multiplier_) :
    m_multiplier{std::move(multiplier_)}
{}

template<typename StateIDT, typename ViewT>
void StateProperties<StateIDT, ViewT>::Update::MultiplyInertia::operator()(const ViewT &view_) const
{
    auto &physical = view_.template get<ComponentPhysical>();
    physical.inertia = physical.inertia.mulComponents(m_multiplier[view_.template cget<SM::StatePossessor<StateIDT>>().framesInState()]);
}


template<typename StateIDT, typename ViewT>
StateProperties<StateIDT, ViewT>::Update::HorizontalVelocityLimit::HorizontalVelocityLimit(TimelineProperty<std::pair<float, float>> &&limits_) :
    m_limits{std::move(limits_)}
{}

template<typename StateIDT, typename ViewT>
void StateProperties<StateIDT, ViewT>::Update::HorizontalVelocityLimit::operator()(const ViewT &view_) const
{
    auto &physical = view_.template get<ComponentPhysical>();
    const auto &currentLimit = m_limits[view_.template cget<SM::StatePossessor<StateIDT>>().framesInState()];
    if (physical.velocity.x < currentLimit.first)
        physical.velocity.x = currentLimit.first;
    else if (physical.velocity.x > currentLimit.second)
        physical.velocity.x = currentLimit.second;
}

template<typename StateIDT, typename ViewT>
StateProperties<StateIDT, ViewT>::Update::VerticalVelocityLimit::VerticalVelocityLimit(TimelineProperty<std::pair<float, float>> &&limits_) :
    m_limits{std::move(limits_)}
{}

template<typename StateIDT, typename ViewT>
void StateProperties<StateIDT, ViewT>::Update::VerticalVelocityLimit::operator()(const ViewT &view_) const
{
    auto &physical = view_.template get<ComponentPhysical>();
    const auto &currentLimit = m_limits[view_.template cget<SM::StatePossessor<StateIDT>>().framesInState()];
    if (physical.m_velocity.y < currentLimit.first)
        physical.m_velocity.y = currentLimit.first;
    else if (physical.m_velocity.y > currentLimit.second)
        physical.m_velocity.y = currentLimit.second;
}

template<typename StateIDT, typename ViewT>
StateProperties<StateIDT, ViewT>::Update::HorizontalInertiaLimit::HorizontalInertiaLimit(TimelineProperty<std::pair<float, float>> &&limits_) :
    m_limits{std::move(limits_)}
{}

template<typename StateIDT, typename ViewT>
void StateProperties<StateIDT, ViewT>::Update::HorizontalInertiaLimit::operator()(const ViewT &view_) const
{
    auto &physical = view_.template get<ComponentPhysical>();
    const auto &currentLimit = m_limits[view_.template cget<SM::StatePossessor<StateIDT>>().framesInState()];
    if (physical.inertia.x < currentLimit.first)
        physical.inertia.x = currentLimit.first;
    else if (physical.inertia.x > currentLimit.second)
        physical.inertia.x = currentLimit.second;
}

template<typename StateIDT, typename ViewT>
void StateProperties<StateIDT, ViewT>::Update::AirDrift::operator()(const ViewT &view_) const
{
    auto &physical = view_.template get<ComponentPhysical>();
    auto &inputs = view_.template get<InputResolver>();
    if (physical.velocity.x < 2.5f
            && inputs.checkInput(InputMotions::HOLD_HORDIR, ORIENTATION::RIGHT, 0))
        physical.velocity.x += 0.15f;

    if (physical.velocity.x > -2.5f
            && inputs.checkInput(InputMotions::HOLD_HORDIR, ORIENTATION::LEFT, 0))
        physical.velocity.x -= 0.15f;

    if (physical.velocity.y < 0
            && view_.template cget<SM::StatePossessor<StateIDT>>().framesInState() < 10
            && inputs.checkInput(InputMotions::HOLD_UP, ORIENTATION::UNSPECIFIED, 0))
        physical.velocity.y -= 0.4f;
}



template<typename StateIDT, typename ViewT>
StateProperties<StateIDT, ViewT>::Update::SetDrag::SetDrag(TimelineProperty<Vector2<float>> &&drag_) :
    m_drag{std::move(drag_)}
{}

template<typename StateIDT, typename ViewT>
void StateProperties<StateIDT, ViewT>::Update::SetDrag::operator()(const ViewT &view_) const
{
    auto &physical = view_.template get<ComponentPhysical>();
    const auto &current = m_drag[view_.template cget<SM::StatePossessor<StateIDT>>().framesInState()];
    physical.drag = current;
}


template<typename StateIDT, typename ViewT>
StateProperties<StateIDT, ViewT>::Update::LookaheadSpeedSensitivity::LookaheadSpeedSensitivity(TimelineProperty<Vector2<float>> &&sensitivity_) :
    m_sensitivity{std::move(sensitivity_)}
{}

template<typename StateIDT, typename ViewT>
void StateProperties<StateIDT, ViewT>::Update::LookaheadSpeedSensitivity::operator()(const ViewT &view_) const
{
    view_.template get<ComponentDynamicCameraTarget>().lookaheadSpeedSensitivity = m_sensitivity[view_.template cget<SM::StatePossessor<StateIDT>>().framesInState()];
}


template<typename StateIDT, typename ViewT>
void StateProperties<StateIDT, ViewT>::Update::TestFallthrough::operator()(const ViewT &view_) const
{
    if (view_.template get<InputResolver>().isInputActive(INPUT_BUTTON::DOWN))
        view_.template get<ComponentObstacleFallthrough>().setIgnoringObstacles();
}


template<typename StateIDT, typename ViewT>
StateProperties<StateIDT, ViewT>::Update::MagnetLimit::MagnetLimit(TimelineProperty<unsigned int> &&magnetLimit_) :
    m_magnetLimit{std::move(magnetLimit_)}
{}

template<typename StateIDT, typename ViewT>
void StateProperties<StateIDT, ViewT>::Update::MagnetLimit::operator()(const ViewT &view_) const
{
    view_.template get<ComponentPhysical>().magnetLimit = m_magnetLimit[view_.template cget<SM::StatePossessor<StateIDT>>().framesInState()];
}


template<typename StateIDT, typename ViewT>
StateProperties<StateIDT, ViewT>::Update::EmitParticles::EmitParticles(ParticleSystem &parSys_, std::map<uint32_t, ParticleEmissionRuleset> &&emission_) :
    m_parSys{parSys_},
    m_emission{std::move(emission_)}
{}

template<typename StateIDT, typename ViewT>
void StateProperties<StateIDT, ViewT>::Update::EmitParticles::operator()(const ViewT &view_) const
{
    const auto frame = view_.template cget<SM::StatePossessor<StateIDT>>().framesInState();
    auto found = m_emission.find(frame);
    if (found == m_emission.end())
        return;

    const auto pid = m_parSys.makeParticle(found->second.recipe, view_);

    if (found->second.mustDestroyOnStateChange)
        view_.template get<ComponentChildParticles>().destroyOnStateChange.emplace_back(pid);
}


template<typename StateIDT, typename ViewT>
StateProperties<StateIDT, ViewT>::Update::Realign::Realign(TimelineProperty<bool> &&shouldRealign_) :
    m_shouldRealign{std::move(shouldRealign_)}
{}

template<typename StateIDT, typename ViewT>
void StateProperties<StateIDT, ViewT>::Update::Realign::operator()(const ViewT &view_) const
{
    if (m_shouldRealign[view_.template cget<SM::StatePossessor<StateIDT>>().framesInState()])
    {
        const auto rawOffset = view_.template get<ComponentPhysical>().peekRawOffset();

        if (rawOffset.x >= 0.001f)
            view_.template get<ComponentTransform>().m_orientation = ORIENTATION::RIGHT;
        else if (rawOffset.x <= -0.001f)
            view_.template get<ComponentTransform>().m_orientation = ORIENTATION::LEFT;
    }
}


template<typename StateIDT, typename ViewT>
StateProperties<StateIDT, ViewT>::Update::CamShake::CamShake(Camera &cam_, std::unordered_map<uint32_t, ShakeRecipe> &&shakes_) :
    m_cam{cam_},
    m_shakes{std::move(shakes_)}
{}

template<typename StateIDT, typename ViewT>
void StateProperties<StateIDT, ViewT>::Update::CamShake::operator()(const ViewT &view_) const
{
    const auto found = m_shakes.find(view_.template cget<SM::StatePossessor<StateIDT>>().framesInState());
    if (found != m_shakes.end())
        m_cam.startShake(found->second.xAmp, found->second.yAmp, found->second.period);
}


template<typename StateIDT, typename ViewT>
StateProperties<StateIDT, ViewT>::Pipe::Notify::Notify(const char *action_) :
    m_action(action_)
{}

template<typename StateIDT, typename ViewT>
void StateProperties<StateIDT, ViewT>::Pipe::Notify::operator()(const ViewT&, const SM::TransitionData<StateIDT> &transition_) const
{
    std::cout << serialize(transition_.fromState) << " => " << serialize(transition_.intoState) << " (" << serialize(transition_.intoOrientation) << "): " << m_action << std::endl;
}

template<typename StateIDT, typename ViewT>
StateProperties<StateIDT, ViewT>::Pipe::SetGravity::SetGravity(const Vector2<float> &gravity_) :
    m_gravity{gravity_}
{}

template<typename StateIDT, typename ViewT>
void StateProperties<StateIDT, ViewT>::Pipe::SetGravity::operator()(const ViewT &view_, const SM::TransitionData<StateIDT>&) const
{
    view_.template get<ComponentPhysical>().gravity = m_gravity;
}

template<typename StateIDT, typename ViewT>
StateProperties<StateIDT, ViewT>::Pipe::SetAnimation::SetAnimation(ResID anim_) :
    m_anim{anim_}
{}

template<typename StateIDT, typename ViewT>
void StateProperties<StateIDT, ViewT>::Pipe::SetAnimation::operator()(const ViewT &view_, const SM::TransitionData<StateIDT>&) const
{
    auto &renderable = view_.template get<ComponentAnimationRenderable>();
    renderable.m_currentAnimation = &renderable.m_animations.at(m_anim);
    renderable.m_currentAnimation->reset();
}

template<typename StateIDT, typename ViewT>
void StateProperties<StateIDT, ViewT>::Pipe::Realign::operator()(const ViewT &view_, const SM::TransitionData<StateIDT> &transition_) const
{
    auto &transform = view_.template get<ComponentTransform>();
    transform.m_orientation = transition_.intoOrientation;
}


template<typename StateIDT, typename ViewT>
StateProperties<StateIDT, ViewT>::Pipe::MultiplyVelocity::MultiplyVelocity(const Vector2<float> &multiplier_) :
    m_multiplier{multiplier_}
{}

template<typename StateIDT, typename ViewT>
void StateProperties<StateIDT, ViewT>::Pipe::MultiplyVelocity::operator()(const ViewT &view_, const SM::TransitionData<StateIDT>&) const
{
    auto &physical = view_.template get<ComponentPhysical>();
    physical.velocity = physical.velocity.mulComponents(m_multiplier);
}

template<typename StateIDT, typename ViewT>
StateProperties<StateIDT, ViewT>::Pipe::AddOrientedVelocity::AddOrientedVelocity(const Vector2<float> &velocity_) :
    m_velocity{velocity_}
{}

template<typename StateIDT, typename ViewT>
void StateProperties<StateIDT, ViewT>::Pipe::AddOrientedVelocity::operator()(const ViewT &view_, const SM::TransitionData<StateIDT>&) const
{
    auto &physical = view_.template get<ComponentPhysical>();
    auto &transform = view_.template get<ComponentTransform>();
    physical.velocity.y += m_velocity.y;
    if (transform.m_orientation == ORIENTATION::RIGHT)
        physical.velocity.x += m_velocity.x;
    else
        physical.velocity.x -= m_velocity.x;
}


template<typename StateIDT, typename ViewT>
StateProperties<StateIDT, ViewT>::Pipe::SetDrag::SetDrag(const Vector2<float> &drag_) :
    m_drag{drag_}
{}

template<typename StateIDT, typename ViewT>
void StateProperties<StateIDT, ViewT>::Pipe::SetDrag::operator()(const ViewT &view_, const SM::TransitionData<StateIDT>&) const
{
    view_.template get<ComponentPhysical>().drag = m_drag;
}


template<typename StateIDT, typename ViewT>
StateProperties<StateIDT, ViewT>::Pipe::ConvertToInertia::ConvertToInertia(bool convertVelocity_, bool includeEnforced_) :
    m_convertVelocity{convertVelocity_},
    m_includeEnforced{includeEnforced_}
{}

template<typename StateIDT, typename ViewT>
void StateProperties<StateIDT, ViewT>::Pipe::ConvertToInertia::operator()(const ViewT &view_, const SM::TransitionData<StateIDT>&) const
{
    view_.template get<ComponentPhysical>().convertToInertia(m_convertVelocity, m_includeEnforced);
}


template<typename StateIDT, typename ViewT>
StateProperties<StateIDT, ViewT>::Pipe::SetInertiaApplicationMultiplier::SetInertiaApplicationMultiplier(const Vector2<float> &mul_) :
    m_mul{mul_}
{}

template<typename StateIDT, typename ViewT>
void StateProperties<StateIDT, ViewT>::Pipe::SetInertiaApplicationMultiplier::operator()(const ViewT &view_, const SM::TransitionData<StateIDT>&) const
{
    view_.template get<ComponentPhysical>().inertiaMultiplier = m_mul;
}


template<typename StateIDT, typename ViewT>
StateProperties<StateIDT, ViewT>::Pipe::SetMagnetLimit::SetMagnetLimit(unsigned int magnetLimit_) :
    m_magnetLimit{magnetLimit_}
{}

template<typename StateIDT, typename ViewT>
void StateProperties<StateIDT, ViewT>::Pipe::SetMagnetLimit::operator()(const ViewT &view_, const SM::TransitionData<StateIDT>&) const
{
    view_.template get<ComponentPhysical>().magnetLimit = m_magnetLimit;
}


template<typename StateIDT, typename ViewT>
void StateProperties<StateIDT, ViewT>::Pipe::TestFallthrough::operator()(const ViewT &view_, const SM::TransitionData<StateIDT>&) const
{
    if (view_.template get<InputResolver>().isInputActive(INPUT_BUTTON::DOWN))
        view_.template get<ComponentObstacleFallthrough>().setIgnoringObstacles();
}

template<typename StateIDT, typename ViewT>
void StateProperties<StateIDT, ViewT>::Pipe::HaltSideDownwardMomentum::operator()(const ViewT &view_, const SM::TransitionData<StateIDT>&) const
{
    auto &physical = view_.template get<ComponentPhysical>();

    if (physical.inertia.y > 0)
        physical.inertia.y = 0;
    if (physical.velocity.y > 0)
        physical.velocity.y = 0;

    physical.velocity.x = 0;
    physical.inertia.x = 0;
}

template<typename StateIDT, typename ViewT>
constexpr StateProperties<StateIDT, ViewT>::Pipe::SetDemandWall::SetDemandWall(bool demandWall_) :
    m_demandWall{demandWall_}
{}

template<typename StateIDT, typename ViewT>
void StateProperties<StateIDT, ViewT>::Pipe::SetDemandWall::operator()(const ViewT &view_, const SM::TransitionData<StateIDT>&) const
{
    view_.template get<WorldPosition>().wall.demand = m_demandWall;
}

template<typename StateIDT, typename ViewT>
void StateProperties<StateIDT, ViewT>::Pipe::LeaveWallPrejump::operator()(const ViewT &view_, const SM::TransitionData<StateIDT>&) const
{
    const auto &inputs = view_.template get<InputResolver>().getHistory();
    const auto &transform = view_.template get<ComponentTransform>();
    auto &physical = view_.template get<ComponentPhysical>();

    const int orient = (transform.m_orientation == ORIENTATION::RIGHT ? 1 : -1);
    Vector2<float> targetSpeed = {orient * 0.7f, 0.1f};;
    bool fall = true;

    const size_t lookAt = std::min(inputs.getFilled() - 1, static_cast<size_t>(gamedata::global::inputBufferLength) * 2);
    for (size_t i = 0; i <= lookAt; ++i)
    {
        const auto &in = inputs[i];

        if (in.m_dir == Vector2{0, -1})
        {
            targetSpeed = {orient * 0.7f, -5.0f};
            fall = false;
            break;
        }

        if (in.m_dir == Vector2{orient, -1})
        {
            targetSpeed = {orient * 1.5f, -4.5f};
            fall = false;
            break;
        }

        if (in.m_dir == Vector2{orient, 0})
        {
            targetSpeed = {orient * 3.0f, -2.2f};
            fall = false;
            break;
        }

        if (in.m_dir == Vector2{orient, 1})
        {
            targetSpeed = {orient * 3.5f, 0.0f};
            fall = false;
            break;
        }

        if (in.m_dir == Vector2{orient, 1})
        {
            break;
        }
    }

    if (fall)
    {
        physical.velocity += targetSpeed;
        physical.inertia = {0.0f, 0.0f};
    }
    else
        physical.velocity += targetSpeed;
}

template<typename StateIDT, typename ViewT>
constexpr StateProperties<StateIDT, ViewT>::Pipe::SetLookaheadSpeedSensitivity::SetLookaheadSpeedSensitivity(const Vector2<float> &sensitivity_) :
    m_sensitivity{sensitivity_}
{}

template<typename StateIDT, typename ViewT>
void StateProperties<StateIDT, ViewT>::Pipe::SetLookaheadSpeedSensitivity::operator()(const ViewT &view_, const SM::TransitionData<StateIDT>&) const
{
    view_.template get<ComponentDynamicCameraTarget>().lookaheadSpeedSensitivity = m_sensitivity;
}


template<typename StateIDT, typename ViewT>
StateProperties<StateIDT, ViewT>::Pipe::DestroyParticlesOnLeave::DestroyParticlesOnLeave(entt::registry &reg_) :
    m_reg{reg_}
{}

template<typename StateIDT, typename ViewT>
void StateProperties<StateIDT, ViewT>::Pipe::DestroyParticlesOnLeave::operator()(const ViewT &view_, const SM::TransitionData<StateIDT>&) const
{
    auto &particles = view_.template get<ComponentChildParticles>().destroyOnStateChange;
    
    for (const auto &el : particles)
    {
        if (m_reg.valid(el))
            m_reg.destroy(el);
    }

    particles.clear();
}
