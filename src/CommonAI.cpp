#include "CommonAI.h"
#include "NavSystem.h"

void AIState::enter(EntityAnywhere owner_, CharState from_)
{
    GenericState::enter(owner_, from_);

    auto &ai = owner_.reg->get<ComponentAI>(owner_.idx);

    ai.m_requestedState = m_enterRequestedState;
    if (m_enterRequestedOrientation.has_value())
        ai.m_requestedOrientation = *m_enterRequestedOrientation;
}

void AIStateNull::enter(EntityAnywhere owner_, CharState from_)
{
    GenericState::enter(owner_, from_);
    auto &ai = owner_.reg->get<ComponentAI>(owner_.idx);
    ai.m_requestedState.reset();
    ai.m_requestedOrientation = ORIENTATION::UNSPECIFIED;
}

AIState &AIState::setEnterRequestedState(std::optional<CharState> enterRequestedState_)
{
    m_enterRequestedState = enterRequestedState_;

    return *this;
}

AIState &AIState::setEnterRequestedOrientation(std::optional<ORIENTATION> enterRequestedOrientation_)
{
    m_enterRequestedOrientation = enterRequestedOrientation_;

    return *this;
}

bool RandomRoamState::update(EntityAnywhere owner_, uint32_t currentFrame_)
{
    NodeState::update(owner_, currentFrame_); 
    
    if (m_timer.update())
    {
        if (m_isWalking)
        {
            //std::cout << "Switching to idle\n";
            m_timer.begin((rand() % (m_idleDuration.second - m_idleDuration.first)) + m_idleDuration.first);
            switchCurrentState(owner_, m_idle);
            m_isWalking = false;
        }
        else
        {
            //std::cout << "Switching to walking\n";

            m_timer.begin((rand() % (m_walkDuration.second - m_walkDuration.first)) + m_walkDuration.first);
            static_cast<AIState*>(m_states[m_stateIds[static_cast<CharState>(m_walk)]].get())->setEnterRequestedOrientation((
                rand() % 2 == 0 ? ORIENTATION::LEFT : ORIENTATION::RIGHT
                ));
            m_isWalking = true;

            switchCurrentState(owner_, m_walk);
        }
    }

    return true;
}

bool BlindChaseState::update(EntityAnywhere owner_, uint32_t currentFrame_)
{
    AIState::update(owner_, currentFrame_); 

    auto &ai = owner_.reg->get<ComponentAI>(owner_.idx);
    const auto &nav = owner_.reg->get<Navigatable>(owner_.idx);
    const auto &ownTrans = owner_.reg->get<ComponentTransform>(owner_.idx);
    const auto &tarTrans = owner_.reg->get<ComponentTransform>(ai.m_chaseTarget);

    if (nav.m_currentOwnConnection)
        if ((nav.m_currentOwnConnection->m_traverses[0] & (1 << Traverse::FallthroughBitID)) || (nav.m_currentOwnConnection->m_traverses[1] & (1 << Traverse::FallthroughBitID)))
            owner_.reg->get<ComponentObstacleFallthrough>(owner_.idx).setIgnoringObstacles();

    auto delta = tarTrans.m_pos - ownTrans.m_pos;
    float range = abs(delta.x);

    if (range <= m_idleRange)
    {
        ai.m_requestedState = m_idle;
    }
    else
    {
        ai.m_requestedState = m_walk;
        ai.m_requestedOrientation = ValueToOrientation(delta.x);
    }

    return true;
}

void BlindChaseState::enter(EntityAnywhere owner_, CharState from_)
{
    AIState::enter(owner_, from_); 

    auto &ai = owner_.reg->get<ComponentAI>(owner_.idx);
    const auto &nav = owner_.reg->get<Navigatable>(owner_.idx);
    const auto &ownTrans = owner_.reg->get<ComponentTransform>(owner_.idx);
    const auto &tarTrans = owner_.reg->get<ComponentTransform>(ai.m_chaseTarget);

    if ((nav.m_currentOwnConnection->m_traverses[0] & (1 << Traverse::FallthroughBitID)) || (nav.m_currentOwnConnection->m_traverses[1] & (1 << Traverse::FallthroughBitID)))
        owner_.reg->get<ComponentObstacleFallthrough>(owner_.idx).setIgnoringObstacles();

    auto delta = tarTrans.m_pos - ownTrans.m_pos;
    float range = delta.getLen();

    if (range <= m_idleRange)
    {
        ai.m_requestedState = m_idle;
    }
    else
    {
        ai.m_requestedState = m_walk;
        ai.m_requestedOrientation = ValueToOrientation(delta.x);
    }

}

bool ProxySelectionState::update(EntityAnywhere owner_, uint32_t currentFrame_)
{
    NodeState::update(owner_, currentFrame_);
    const auto &ownTrans = owner_.reg->get<ComponentTransform>(owner_.idx);
    const auto &tarTrans = m_target.reg->get<ComponentTransform>(m_target.idx);
    auto range = (tarTrans.m_pos - ownTrans.m_pos).getLen();

    for (size_t i = 0; i < m_rangeLimits.size(); ++i)
    {
        if (range <= m_rangeLimits[i])
        {
            if (m_currentState->m_stateId != m_states[i])
                switchCurrentState(owner_, m_states[i]);

            return true;
        }
    }

    if (m_currentState->m_stateId != m_states.back())
        switchCurrentState(owner_, m_states.back());

    return true;
}

void MoveTowards::enter(EntityAnywhere owner_, CharState from_)
{
    AIState::enter(owner_, from_);

    auto &ai = owner_.reg->get<ComponentAI>(owner_.idx);
    const auto &trans = owner_.reg->get<ComponentTransform>(owner_.idx);
    const auto &phys = owner_.reg->get<ComponentPhysical>(owner_.idx);

    auto pb = phys.m_pushbox + trans.m_pos;

    if (!owner_.reg->get<ComponentObstacleFallthrough>(owner_.idx).m_ignoredObstacles.empty())
        ai.m_requestedOrientation = ValueToOrientation(ai.m_navigationTarget.x - trans.m_pos.x);

    ai.m_requestedState = m_walk;
}

bool MoveTowards::update(EntityAnywhere owner_, uint32_t currentFrame_)
{
    AIState::update(owner_, currentFrame_); 

    auto &ai = owner_.reg->get<ComponentAI>(owner_.idx);
    const auto &trans = owner_.reg->get<ComponentTransform>(owner_.idx);
    const auto &phys = owner_.reg->get<ComponentPhysical>(owner_.idx);
    auto delta = ai.m_navigationTarget - trans.m_pos;

    ai.m_requestedState = m_walk;
    //std::cout << "m_lastPos: " << m_lastPos << std::endl;

    if (owner_.reg->get<ComponentObstacleFallthrough>(owner_.idx).m_ignoredObstacles.empty())
    {
        ai.m_allowLeaveState = true;
        ai.m_requestedOrientation = ValueToOrientation(ai.m_navigationTarget.x - trans.m_pos.x);
        //std::cout << "Updating target orientation\n";
    }
    else
    {
        ai.m_allowLeaveState = false;
        //std::cout << "Overlapping with obstracle\n";
    }

    //std::cout << "d(" << delta << ") " << phys.m_velocity << std::endl;

    return true;
}

bool NavigateGraphChase::update(EntityAnywhere owner_, uint32_t currentFrame_)
{
    NodeState::update(owner_, currentFrame_); 

    auto &nav = owner_.reg->get<Navigatable>(owner_.idx);
    const auto &trans = owner_.reg->get<ComponentTransform>(owner_.idx);
    const auto &phys = owner_.reg->get<ComponentPhysical>(owner_.idx);
    auto pb = phys.m_pushbox + trans.m_pos;

    // TODO: should be checked and set upon entering state
    if (!nav.m_currentPath)
    {
        nav.m_currentPath = owner_.reg->get<World>(owner_.idx).getNavsys().makePath(nav.m_traverseTraits, owner_.reg->get<ComponentAI>(owner_.idx).m_chaseTarget);
    }

    if (nav.m_currentOwnConnection)
        nav.m_currentPath->buildUntil(nav.m_currentOwnConnection);
    else
        // Failed to identify current connection - probably too far from all connections
        return true;

    const auto *currentcon = &nav.m_currentPath->m_graphView[nav.m_currentOwnConnection->m_ownId];
    
    // Check if can start moving along next connection
    if ((!nav.m_checkIfGrounded || (phys.m_onGround != entt::null)) &&
        *currentcon->m_nextConnection &&
        currentcon->m_nextConnection != currentcon &&
        pb.includesPoint(nav.m_currentPath->m_graph.getNodePos(currentcon->m_originalCon.m_nodes[currentcon->m_nextNode])))
    {
        //std::cout << "Overriding connection" << std::endl;
        currentcon = *currentcon->m_nextConnection;
        nav.m_currentOwnConnection = &currentcon->m_originalCon;
    }

    auto &ai = owner_.reg->get<ComponentAI>(owner_.idx);
    if (currentcon->m_nextConnection == currentcon)
    {
        if (ai.m_allowLeaveState)
            if (m_currentState->m_stateId != m_onSuccess)
                switchCurrentState(owner_, m_onSuccess);
        
        return true; 
    }

    if (currentcon->m_nextConnection == nullptr)
    {
        if (ai.m_allowLeaveState)
            if (m_currentState->m_stateId != m_noConnection)
                switchCurrentState(owner_, m_noConnection);
        return true;
    }

    const auto nextNodePos = nav.m_currentPath->m_graph.getNodePos(currentcon->m_originalCon.m_nodes[currentcon->m_nextNode]);
    ai.m_navigationTarget = nextNodePos;
    if (currentcon->m_originalCon.m_traverses[1 - currentcon->m_nextNode] & (1 << Traverse::FallthroughBitID))
        owner_.reg->get<ComponentObstacleFallthrough>(owner_.idx).setIgnoringObstacles();

    const auto &traverse = currentcon->m_originalCon.m_traverses[1 - currentcon->m_nextNode];

    if ((phys.m_onGround != entt::null) && (((traverse >> Traverse::ReservedBits) & static_cast<Traverse::TraitT>(TraverseTraits::FALL)) ||
         ((traverse >> Traverse::ReservedBits) & static_cast<Traverse::TraitT>(TraverseTraits::WALK))))
    {
        if (m_currentState->m_stateId != m_moveTowards)
            switchCurrentState(owner_, m_moveTowards);
    }
    else if ((traverse >> Traverse::ReservedBits) & static_cast<Traverse::TraitT>(TraverseTraits::JUMP))
    {
        if (m_currentState->m_stateId != m_jumpTowards)
            switchCurrentState(owner_, m_jumpTowards);
    }
    else
    {
        if (ai.m_allowLeaveState && m_currentState->m_stateId != m_noConnection)
            switchCurrentState(owner_, m_noConnection);
    }

    return true;
}

bool JumpTowards::update(EntityAnywhere owner_, uint32_t currentFrame_)
{
    AIState::update(owner_, currentFrame_); 

    auto &ai = owner_.reg->get<ComponentAI>(owner_.idx);
    const auto &trans = owner_.reg->get<ComponentTransform>(owner_.idx);

    ai.m_requestedState = m_prejump;
    ai.m_requestedOrientation = ValueToOrientation(ai.m_navigationTarget.x - trans.m_pos.x);

    return true;
}
