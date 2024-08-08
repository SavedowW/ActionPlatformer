#include "StateMachine.h"

GenericState *StateMachine::getRealCurrentState()
{
    return m_currentState->getRealCurrentState();
}

void StateMachine::addState(std::unique_ptr<GenericState> &&state_)
{
    m_stateIds[state_->m_stateId] = m_states.size();
    state_->setParent(this);
    m_states.push_back(std::move(state_));
}

void StateMachine::switchCurrentState(EntityAnywhere owner_, GenericState *state_)
{
    m_currentState->leave(owner_, state_->m_stateId);
    state_->enter(owner_, m_currentState->m_stateId);
    m_currentState = state_;
    m_framesInState = 0;
}

bool StateMachine::update(EntityAnywhere owner_, uint32_t currentFrame_)
{
    auto updres = m_currentState->update(owner_, m_framesInState);
    auto *untilst = (updres ? nullptr : m_currentState);

    if (!attemptTransition(owner_, untilst))
    {
        m_framesInState++;
    }
    else
        return true;

    return false;
}

bool StateMachine::attemptTransition(EntityAnywhere owner_, GenericState* until_)
{
    auto currentStateId = m_currentState->m_stateId;
    for (auto &el : m_states)
    {
        if (el->transitionableFrom(currentStateId))
        {
            auto res = el->isPossible(owner_);
            if (res != ORIENTATION::UNSPECIFIED)
            {
                auto &trans = owner_.reg->get<ComponentTransform>(owner_.idx);
                trans.m_orientation = res;
                switchCurrentState(owner_, el.get());
                return true;
            }
        }

        if (el.get() == until_)
            return false;
    }

    return false;
}

std::string StateMachine::getName() const
{
    return std::string("root") + " -> " + m_currentState->getName(m_framesInState);
}

std::ostream &operator<<(std::ostream &os_, const StateMachine &rhs_)
{
    os_ << rhs_.getName();
    return os_;
}

void GenericState::setParent(StateMachine *parent_)
{
    m_parent = parent_;
}

GenericState *GenericState::getRealCurrentState()
{
    return this;
}

GenericState &GenericState::setGravity(TimelineProperty<Vector2<float>> &&gravity_)
{
    m_gravity = std::move(gravity_);
    return *this;
}

GenericState &GenericState::setDrag(TimelineProperty<Vector2<float>> &&drag_)
{
    m_drag = std::move(drag_);
    return *this;
}

GenericState &GenericState::setCanFallThrough(TimelineProperty<bool> &&fallThrough_)
{
    m_canFallThrough = std::move(fallThrough_);
    return *this;
}

GenericState &GenericState::setNoLanding(TimelineProperty<bool> &&noLanding_)
{
    m_noUpwardLanding = std::move(noLanding_);
    return *this;
}

GenericState &GenericState::setAppliedInertiaMultiplier(TimelineProperty<Vector2<float>> &&inerMul_)
{
    m_appliedInertiaMultiplier = std::move(inerMul_);
    return *this;
}

GenericState &GenericState::setConvertVelocityOnSwitch(bool convertVelocity_)
{
    m_convertVelocityOnSwitch = convertVelocity_;
    return *this;
}

GenericState &GenericState::setUpdateMovementData(TimelineProperty<Vector2<float>> &&mulOwnVelUpd_, TimelineProperty<Vector2<float>> &&mulOwnDirVelUpd_, TimelineProperty<Vector2<float>> &&rawAddVelUpd_, TimelineProperty<Vector2<float>> &&mulOwnInrUpd_, TimelineProperty<Vector2<float>> &&mulOwnDirInrUpd_, TimelineProperty<Vector2<float>> &&rawAddInrUpd_)
{
    m_mulOwnVelUpd = std::move(mulOwnVelUpd_);
    m_mulOwnDirVelUpd = std::move(mulOwnDirVelUpd_);
    m_rawAddVelUpd = std::move(rawAddVelUpd_);
    m_mulOwnInrUpd = std::move(mulOwnInrUpd_);
    m_mulOwnDirInrUpd = std::move(mulOwnDirInrUpd_);
    m_rawAddInrUpd = std::move(rawAddInrUpd_);

    m_usingUpdateMovement = !m_mulOwnVelUpd.isEmpty() || !m_mulOwnDirVelUpd.isEmpty() || !m_rawAddVelUpd.isEmpty()
                        || !m_mulOwnInrUpd.isEmpty() || !m_mulOwnDirInrUpd.isEmpty() || !m_rawAddInrUpd.isEmpty();

    return *this;
}

GenericState &GenericState::setMagnetLimit(TimelineProperty<float> &&magnetLimit_)
{
    m_magnetLimit = std::move(magnetLimit_);
    return *this;
}

GenericState &GenericState::setUpdateSpeedLimitData(TimelineProperty<Vector2<float>> &&ownVelLimitUpd_, TimelineProperty<Vector2<float>> &&ownInrLimitUpd_)
{
    m_ownVelLimitUpd = std::move(ownVelLimitUpd_);
    m_ownInrLimitUpd = std::move(ownInrLimitUpd_);

    return *this;
}

GenericState &GenericState::setGroundedOnSwitch(bool isGrounded_)
{
    m_setGroundedOnSwitch = isGrounded_;
    return *this;
}

GenericState &GenericState::setCooldown(FrameTimer<true> *cooldown_, int cooldownTime_)
{
    m_cooldown = cooldown_;
    m_cooldownTime = cooldownTime_;
    return *this;
}

GenericState &GenericState::setRecoveryFrames(TimelineProperty<StateMarker> &&recoveryFrames_)
{
    m_recoveryFrames = std::move(recoveryFrames_);
    return *this;
}

void GenericState::enter(EntityAnywhere owner_, CharState from_)
{
    std::cout << "Switched to " << m_stateName << std::endl;

    auto &renderable = owner_.reg->get<ComponentAnimationRenderable>(owner_.idx);
    auto &physical = owner_.reg->get<ComponentPhysical>(owner_.idx);

    // Handle animation
    if (m_uniqueTransitionAnims.contains(from_))
        renderable.m_currentAnimation = renderable.m_animations[m_uniqueTransitionAnims[from_]].get();
    else
        renderable.m_currentAnimation = renderable.m_animations[m_anim].get();
    renderable.m_currentAnimation->reset();

    // Convert velocity
    if (m_convertVelocityOnSwitch)
        physical.velocityToInertia();

    // Force grounded
    if (m_setGroundedOnSwitch.isSet())
        physical.m_isGrounded = m_setGroundedOnSwitch;

    // set cooldown if necessary
    if (m_cooldown)
        m_cooldown->begin(m_cooldownTime);

    //physical.m_pushbox = Collider{Vector2{0.0f, -30.0f}, Vector2{10.0f, 30.0f}}; // TODO: to property

    physical.m_noUpwardLanding = m_noUpwardLanding[0];
    physical.m_magnetLimit = m_magnetLimit[0];
}

void GenericState::leave(EntityAnywhere owner_, CharState to_)
{
}

bool GenericState::update(EntityAnywhere owner_, uint32_t currentFrame_)
{
    auto &transform = owner_.reg->get<ComponentTransform>(owner_.idx);
    auto &physical = owner_.reg->get<ComponentPhysical>(owner_.idx);
    auto &animrnd = owner_.reg->get<ComponentAnimationRenderable>(owner_.idx);

    animrnd.m_currentAnimation->update();

    // Handle velocity and inertia changes
    if (m_usingUpdateMovement)
    {
        physical.m_velocity = physical.m_velocity.mulComponents(m_mulOwnVelUpd[currentFrame_]) + Vector2<int>(transform.m_orientation).mulComponents(m_mulOwnDirVelUpd[currentFrame_]) + m_rawAddVelUpd[currentFrame_];
        physical.m_inertia = physical.m_inertia.mulComponents(m_mulOwnInrUpd[currentFrame_]) + Vector2<int>(transform.m_orientation).mulComponents(m_mulOwnDirInrUpd[currentFrame_]) + m_rawAddInrUpd[currentFrame_];
    }

    // Handle gravity
    physical.m_gravity = m_gravity[currentFrame_];
    physical.m_drag = m_drag[currentFrame_];
    physical.m_inertiaMultiplier = m_appliedInertiaMultiplier[currentFrame_];

    // Handle velocity and inertia limits
    if (!m_ownVelLimitUpd.isEmpty())
        physical.m_velocity = utils::clamp(physical.m_velocity, -m_ownVelLimitUpd[currentFrame_], m_ownVelLimitUpd[currentFrame_]);

    if (!m_ownInrLimitUpd.isEmpty())
        physical.m_inertia = utils::clamp(physical.m_inertia, -m_ownInrLimitUpd[currentFrame_], m_ownInrLimitUpd[currentFrame_]);

    physical.m_noUpwardLanding = m_noUpwardLanding[currentFrame_];
    physical.m_magnetLimit = m_magnetLimit[currentFrame_];

    // Handle duration
    if (m_transitionOnOutdated.isSet())
    {
        if (currentFrame_ >= m_duration)
            onOutdated(owner_);
    }

    return true;
}

ORIENTATION GenericState::isPossible(EntityAnywhere owner_) const
{
    return owner_.reg->get<ComponentTransform>(owner_.idx).m_orientation;
}

std::string GenericState::getName(uint32_t framesInState_) const
{
    return m_stateName + " (" + std::to_string(framesInState_) + ")";
}

void GenericState::onOutdated(EntityAnywhere owner_)
{
    m_parent->switchCurrentState(owner_, m_transitionOnOutdated);
}

void GenericState::onTouchedGround(EntityAnywhere owner_)
{
    if (m_transitionOnLand.isSet())
    {
        m_parent->switchCurrentState(owner_, m_transitionOnLand);
    }
}

void GenericState::onLostGround(EntityAnywhere owner_)
{
    if (m_transitionOnLostGround.isSet())
    {
        m_parent->switchCurrentState(owner_, m_transitionOnLostGround);
    }
}

std::string NodeState::getName(uint32_t framesInState_) const
{
    return std::string(GenericState::m_stateName) + " (" + std::to_string(framesInState_) + ") -> " + StateMachine::m_currentState->getName(StateMachine::m_framesInState);
}

bool NodeState::update(EntityAnywhere owner_, uint32_t currentFrame_)
{
    GenericState::update(owner_, currentFrame_);
    return StateMachine::update(owner_, currentFrame_);
}

GenericState *NodeState::getRealCurrentState()
{
    return StateMachine::m_currentState->getRealCurrentState();
}