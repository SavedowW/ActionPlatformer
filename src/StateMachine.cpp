#include "StateMachine.hpp"
#include "Core/Application.h"

GenericState *StateMachine::getRealCurrentState()
{
    return m_currentState->getRealCurrentState();
}

void StateMachine::switchCurrentState(EntityAnywhere owner_, GenericState *state_)
{
    m_currentState->leave(owner_, state_->m_stateId);
    state_->enter(owner_, m_currentState->m_stateId);
    m_currentState = state_;
    m_timeInState = Time::NS{0};
}

bool StateMachine::update(EntityAnywhere owner_, const Time::NS&)
{
    m_timeInState += Application::instance().timestep.getFrameDuration();
    auto updres = m_currentState->update(owner_, m_timeInState);
    auto *untilst = (updres ? nullptr : m_currentState);

    if (attemptTransition(owner_, untilst))
        return true;

    return false;
}

bool StateMachine::attemptTransition(EntityAnywhere owner_, GenericState* until_)
{
    auto currentStateId = m_currentState->m_stateId;
    for (auto &el : m_states)
    {
        if (el->transitionableFrom(currentStateId) || m_currentState->transitionableInto(el->m_stateId, m_timeInState))
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
    return std::string("root") + " -> " + m_currentState->getName(m_timeInState);
}

void StateMachine::switchCurrentState(EntityAnywhere owner_, const std::vector<CharState>::iterator &current_,
    const std::vector<CharState>::iterator &end_)
{
    switchCurrentState(owner_, *current_);
    auto next = current_ + 1;
    if (next != end_)
    {
        auto *nextSm = dynamic_cast<StateMachine*>(m_currentState);
        assert(nextSm);
        nextSm->switchCurrentState(owner_, next, end_);
    }
}

void GenericState::spawnParticle(EntityAnywhere owner_, const ParticleTemplate &partemplate_, const ComponentTransform &trans_, const ComponentPhysical &phys_, World &world_, SDL_FlipMode  flip_)
{
    ParticleRecipe rp(partemplate_);

    if (partemplate_.m_tiePosRule != TiePosRule::TIE_TO_SOURCE)
        rp.pos = trans_.m_pos;

    if (trans_.m_orientation == ORIENTATION::LEFT || trans_.m_orientation == ORIENTATION::RIGHT && (flip_ & SDL_FLIP_HORIZONTAL))
    {
        rp.flip = SDL_FlipMode (rp.flip | SDL_FLIP_HORIZONTAL);
        rp.pos.x -= partemplate_.offset.x;

        if (!(flip_ & SDL_FLIP_VERTICAL) && partemplate_.m_dependOnGroundAngle)
            rp.pos.y -= partemplate_.offset.x * (partemplate_.m_dependOnGroundAngle ? phys_.m_onSlopeWithAngle : 1.0f);
    }
    else
    {
        rp.pos.x += partemplate_.offset.x;

        if (!(flip_ & SDL_FLIP_VERTICAL) && partemplate_.m_dependOnGroundAngle)
            rp.pos.y += partemplate_.offset.x * (partemplate_.m_dependOnGroundAngle ? phys_.m_onSlopeWithAngle : 1.0f);
    }

    if (flip_ & SDL_FLIP_VERTICAL)
    {
        rp.flip = SDL_FlipMode (rp.flip | SDL_FLIP_VERTICAL);
        rp.pos.y -= partemplate_.offset.y;
    }
    else
        rp.pos.y += partemplate_.offset.y;

    if (partemplate_.m_tiePosRule == TiePosRule::TIE_TO_GROUND)
    {
        rp.m_tiePosTo = phys_.m_onGround;

        if (partemplate_.m_dependOnGroundAngle)
            rp.angle = atan(phys_.m_onSlopeWithAngle) * 180 / 3.1415f;
    }
    else if (partemplate_.m_tiePosRule == TiePosRule::TIE_TO_WALL)
    {
        rp.m_tiePosTo = phys_.m_onWall;
    }
    else if (partemplate_.m_tiePosRule == TiePosRule::TIE_TO_SOURCE)
    {
        rp.m_tiePosTo = owner_.idx;
    }

    world_.getParticleSys().makeParticle(rp, (partemplate_.m_tieLifetimeRule == TieLifetimeRule::DESTROY_ON_STATE_LEAVE ? &m_lifetimeTiedParticles : nullptr));
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

bool GenericState::transitionableFrom(CharState targetStateId_) const
{
    return m_transitionableFrom[targetStateId_];
}

bool GenericState::transitionableInto(CharState, const Time::NS&) const
{
    return false;
}

void PhysicalState::updateActor(BattleActor &battleActor_) const
{
    battleActor_.m_timeInState = m_parent->m_timeInState;

    if (m_hasHurtboxes)
        battleActor_.m_hurtboxes = &m_hurtboxes;
    else
        battleActor_.m_hurtboxes = nullptr;

    battleActor_.m_activeHits.clear();
    for (const auto &hit : m_hits)
    {
        if (battleActor_.m_timeInState >= hit.m_activeWindow.first && battleActor_.m_timeInState <= hit.m_activeWindow.second)
        {
            battleActor_.m_activeHits.push_back(&hit);
        }
    }

    if (m_hitStateMapping.has_value())
        battleActor_.m_hitStateTransitions = &(m_hitStateMapping.value());
    else
        battleActor_.m_hitStateTransitions = nullptr;
}

PhysicalState &PhysicalState::setGravity(TimelineProperty<Time::NS, Vector2<float>> &&gravity_)
{
    m_gravity = std::move(gravity_);
    return *this;
}

PhysicalState &PhysicalState::setDrag(TimelineProperty<Time::NS, Vector2<float>> &&drag_)
{
    m_drag = std::move(drag_);
    return *this;
}

PhysicalState &PhysicalState::setMulInsidePushbox(TimelineProperty<Time::NS, std::optional<Vector2<float>>> &&mulInsidePushbox_)
{
    m_mulInsidePushbox = std::move(mulInsidePushbox_);
    return *this;
}

PhysicalState &PhysicalState::setCanFallThrough(TimelineProperty<Time::NS, bool> &&fallThrough_)
{
    m_canFallThrough = std::move(fallThrough_);
    return *this;
}

PhysicalState &PhysicalState::setNoLanding(TimelineProperty<Time::NS, bool> &&noLanding_)
{
    m_noLanding = std::move(noLanding_);
    return *this;
}

PhysicalState &PhysicalState::setAppliedInertiaMultiplier(TimelineProperty<Time::NS, Vector2<float>> &&inerMul_)
{
    m_appliedInertiaMultiplier = std::move(inerMul_);
    return *this;
}

PhysicalState &PhysicalState::setTransitionVelocityMultiplier(TimelineProperty<Time::NS, Vector2<float>> &&convRate_)
{
    m_transitionVelocityMultiplier = std::move(convRate_);
    return *this;
}

PhysicalState &PhysicalState::setConvertVelocityOnSwitch(bool convertVelocity_, bool convertEnforced_)
{
    m_convertVelocityOnSwitch = convertVelocity_;
    m_convertEnforcedVelocity = convertEnforced_;
    return *this;
}

PhysicalState &PhysicalState::setUpdateMovementData(TimelineProperty<Time::NS, Vector2<float>> &&mulOwnVelUpd_, TimelineProperty<Time::NS, Vector2<float>> &&mulOwnDirVelUpd_, TimelineProperty<Time::NS, Vector2<float>> &&rawAddVelUpd_, TimelineProperty<Time::NS, Vector2<float>> &&mulOwnInrUpd_, TimelineProperty<Time::NS, Vector2<float>> &&mulOwnDirInrUpd_, TimelineProperty<Time::NS, Vector2<float>> &&rawAddInrUpd_)
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

PhysicalState &PhysicalState::setMagnetLimit(TimelineProperty<Time::NS, unsigned int> &&magnetLimit_)
{
    m_magnetLimit = std::move(magnetLimit_);
    return *this;
}

PhysicalState &PhysicalState::setUpdateSpeedLimitData(TimelineProperty<Time::NS, Vector2<float>> &&ownVelLimitUpd_, TimelineProperty<Time::NS, Vector2<float>> &&ownInrLimitUpd_)
{
    m_ownVelLimitUpd = std::move(ownVelLimitUpd_);
    m_ownInrLimitUpd = std::move(ownInrLimitUpd_);

    return *this;
}

PhysicalState &PhysicalState::setRecoveryFrames(TimelineProperty<Time::NS, StateMarker> &&recoveryFrames_)
{
    m_recoveryFrames = std::move(recoveryFrames_);
    return *this;
}

PhysicalState &PhysicalState::setHurtboxes(Hurtbox &&hurtboxes_)
{
    m_hasHurtboxes = true;
    m_hurtboxes = std::move(hurtboxes_);
    return *this;
}

PhysicalState &PhysicalState::addHit(HitboxGroup &&hit_)
{
    m_hits.push_back(std::move(hit_));
    return *this;
}

PhysicalState &PhysicalState::setHitStateMapping(HitStateMapping &&hitStateMapping_)
{
    m_hitStateMapping = std::move(hitStateMapping_);
    return *this;
}

bool PhysicalState::transitionableInto(CharState targetStateId_, const Time::NS &timeInState_) const
{
    return !m_recoveryFrames.isEmpty() && m_recoveryFrames[timeInState_][targetStateId_];
}

GenericState &GenericState::setParticlesSingle(TimelineProperty<Time::NS, ParticleTemplate> &&particlesSingle_)
{
    m_particlesSingle = std::move(particlesSingle_);
    return *this;
}

GenericState &GenericState::setParticlesLoopable(TimelineProperty<Time::NS, ParticleTemplate> &&particlesLoopable_, const Time::NS &loopDuration_)
{
    m_particlesLoopable = std::move(particlesLoopable_);
    m_loopDuration = loopDuration_;
    return *this;
}

void GenericState::enter(EntityAnywhere, CharState)
{
    //std::cout << "Switched to " << m_stateName << std::endl;
}

void PhysicalState::leave(EntityAnywhere owner_, CharState to_)
{
    auto &physical = owner_.reg->get<ComponentPhysical>(owner_.idx);
    physical.m_velocity = physical.m_velocity.mulComponents(physical.m_stateLeaveVelocityMultiplier);

    GenericState::leave(owner_, to_);
}

void PhysicalState::enter(EntityAnywhere owner_, CharState from_)
{
    GenericState::enter(owner_, from_);

    auto [renderable, physical] = owner_.reg->get<ComponentAnimationRenderable, ComponentPhysical>(owner_.idx);

    // Handle animation
    if (m_uniqueTransitionAnims.contains(from_))
        renderable.m_currentAnimation = &renderable.m_animations.at(m_uniqueTransitionAnims[from_]);
    else
        renderable.m_currentAnimation = &renderable.m_animations.at(m_anim);
    renderable.m_currentAnimation->reset();

    // Convert velocity
    if (m_convertVelocityOnSwitch || m_convertEnforcedVelocity)
        physical.convertToInertia(m_convertVelocityOnSwitch, m_convertEnforcedVelocity);

    physical.m_drag = m_drag[Time::NS{0}];
    physical.m_noLanding = m_noLanding[Time::NS{0}];
    physical.m_magnetLimit = m_magnetLimit[Time::NS{0}];

    for (auto &el : m_hits)
    {
        el.m_hitData.updateId();
    }
}

void GenericState::leave(EntityAnywhere owner_, CharState)
{
    for (auto &particle : m_lifetimeTiedParticles)
    {
        if (owner_.reg->valid(particle))
            owner_.reg->destroy(particle);
    }

    m_lifetimeTiedParticles.clear();
}

bool GenericState::update(EntityAnywhere owner_, const Time::NS &timeInState_)
{
    const auto &partemplate = m_particlesSingle[timeInState_];
    const auto &partemplateLoop = m_particlesLoopable[timeInState_ % m_loopDuration];
    if (partemplate.count || partemplateLoop.count)
    {
        auto &&tpl = owner_.reg->get<ComponentTransform, ComponentPhysical, World>(owner_.idx);
        const auto &transform = std::get<0>(tpl);
        const auto &phys = std::get<1>(tpl);
        auto &world = std::get<2>(tpl);
        
        if (partemplate.count)
            spawnParticle(owner_, partemplate, transform, phys, world, SDL_FLIP_NONE);

        if (partemplateLoop.count)
            spawnParticle(owner_, partemplateLoop, transform, phys, world, SDL_FLIP_NONE);
    }

    // Handle duration
    if (m_transitionOnOutdated.has_value())
    {
        if (timeInState_ >= m_duration)
            onOutdated(owner_);
    }

    return true;
}

bool PhysicalState::update(EntityAnywhere owner_, const Time::NS &timeInState_)
{
    auto &&tpl = owner_.reg->get<ComponentTransform, ComponentPhysical>(owner_.idx);
    const auto &transform = std::get<0>(tpl);
    auto &physical = std::get<1>(tpl);

    // Handle velocity and inertia changes
    if (m_usingUpdateMovement)
    {
        physical.m_velocity = physical.m_velocity.mulComponents(m_mulOwnVelUpd[timeInState_]) + Vector2<int>(transform.m_orientation).mulComponents(m_mulOwnDirVelUpd[timeInState_]) + m_rawAddVelUpd[timeInState_];
        physical.m_inertia = physical.m_inertia.mulComponents(m_mulOwnInrUpd[timeInState_]) + Vector2<int>(transform.m_orientation).mulComponents(m_mulOwnDirInrUpd[timeInState_]) + m_rawAddInrUpd[timeInState_];
    }

    // Handle velocity and inertia limits
    if (!m_ownVelLimitUpd.isEmpty())
        physical.m_velocity = utils::clamp(physical.m_velocity, -m_ownVelLimitUpd[timeInState_], m_ownVelLimitUpd[timeInState_]);

    if (!m_ownInrLimitUpd.isEmpty())
        physical.m_inertia = utils::clamp(physical.m_inertia, -m_ownInrLimitUpd[timeInState_], m_ownInrLimitUpd[timeInState_]);

    // Offset multiplier inside pushbox
    if (m_mulInsidePushbox[timeInState_])
        physical.m_mulInsidePushbox = &m_mulInsidePushbox[timeInState_].value();
    else
        physical.m_mulInsidePushbox = nullptr;

    physical.m_stateLeaveVelocityMultiplier = m_transitionVelocityMultiplier[timeInState_];

    // Handle gravity
    physical.m_gravity = m_gravity[timeInState_];
    physical.m_drag = m_drag[timeInState_];
    physical.m_inertiaMultiplier = m_appliedInertiaMultiplier[timeInState_];
    physical.m_noLanding = m_noLanding[timeInState_];
    physical.m_magnetLimit = m_magnetLimit[timeInState_];

    return GenericState::update(owner_, timeInState_);
}

ORIENTATION GenericState::isPossible(EntityAnywhere owner_) const
{
    return owner_.reg->get<ComponentTransform>(owner_.idx).m_orientation;
}

std::string GenericState::getName(const Time::NS &timeInState_) const
{
    return m_stateName + " (" + std::string(timeInState_) + ")";
}

void GenericState::onOutdated(EntityAnywhere owner_)
{
    if (m_transitionOnOutdated.has_value())
    {
        m_parent->switchCurrentState(owner_, *m_transitionOnOutdated);
    }
}

void PhysicalState::onTouchedGround(EntityAnywhere owner_)
{
    const auto &transition = m_transitionsOnLand[m_parent->m_timeInState];
    if (transition.has_value())
    {
        m_parent->switchCurrentState(owner_, *transition);
    }
}

void PhysicalState::onLostGround(EntityAnywhere owner_)
{
    owner_.reg->get<ComponentPhysical>(owner_.idx).m_onMovingPlatform = false;

    if (m_transitionOnLostGround.has_value())
    {
        m_parent->switchCurrentState(owner_, *m_transitionOnLostGround);
    }
}

std::string NodeState::getName(const Time::NS &timeInState_) const
{
    return std::string(GenericState::m_stateName) + " (" + std::string(timeInState_) + ") -> " + StateMachine::m_currentState->getName(StateMachine::m_timeInState);
}

bool NodeState::update(EntityAnywhere owner_, const Time::NS &timeInState_)
{
    GenericState::update(owner_, timeInState_);
    return StateMachine::update(owner_, timeInState_);
}

GenericState *NodeState::getRealCurrentState()
{
    return StateMachine::m_currentState->getRealCurrentState();
}