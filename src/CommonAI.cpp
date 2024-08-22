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
    auto &ownTrans = owner_.reg->get<ComponentTransform>(owner_.idx);
    auto &tarTrans = owner_.reg->get<ComponentTransform>(m_target.idx);

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

    return true;
}

bool ProxySelectionState::update(EntityAnywhere owner_, uint32_t currentFrame_)
{
    NodeState::update(owner_, currentFrame_);
    auto &ownTrans = owner_.reg->get<ComponentTransform>(owner_.idx);
    auto &tarTrans = m_target.reg->get<ComponentTransform>(m_target.idx);
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

bool MoveTowards::update(EntityAnywhere owner_, uint32_t currentFrame_)
{
    AIState::update(owner_, currentFrame_); 

    auto &ai = owner_.reg->get<ComponentAI>(owner_.idx);
    auto &trans = owner_.reg->get<ComponentTransform>(owner_.idx);

    ai.m_requestedState = m_walk;
    if (m_lastPos != ai.m_navigationTarget)
    {
        m_lastPos = ai.m_navigationTarget;
        auto pb = owner_.reg->get<ComponentPhysical>(owner_.idx).m_pushbox + trans.m_pos;

        if (!owner_.reg->get<World>(owner_.idx).isOverlappingObstacle(pb))
            ai.m_requestedOrientation = ValueToOrientation(ai.m_navigationTarget.x - trans.m_pos.x);
    }

    return true;
}

bool NavigateGraphChase::update(EntityAnywhere owner_, uint32_t currentFrame_)
{
    NodeState::update(owner_, currentFrame_); 

    auto &nav = owner_.reg->get<Navigatable>(owner_.idx);
    auto &trans = owner_.reg->get<ComponentTransform>(owner_.idx);
    auto pb = owner_.reg->get<ComponentPhysical>(owner_.idx).m_pushbox + trans.m_pos;

    if (!nav.m_currentPath)
    {
        nav.m_currentPath = owner_.reg->get<World>(owner_.idx).getNavsys().makePath(nav.m_traverseTraits, owner_.reg->get<ComponentAI>(owner_.idx).m_chaseTarget);
    }

    if (nav.m_currentOwnConnection)
        nav.m_currentPath->buildUntil(nav.m_currentOwnConnection);

    if (!nav.m_currentOwnConnection)
        return true;

    auto *currentcon = &nav.m_currentPath->m_fullGraph[nav.m_currentOwnConnection->m_ownId];
    
    if (*currentcon->m_nextConnection && currentcon->m_nextConnection != currentcon && pb.includesPoint(nav.m_currentPath->m_graph->getNodePos(currentcon->m_con->m_nodes[currentcon->m_nextNode])))
        currentcon = *currentcon->m_nextConnection;

    if (currentcon->m_nextConnection == currentcon)
    {
        if (m_currentState->m_stateId != m_noConnection)
            switchCurrentState(owner_, m_noConnection);
        return true;
    }

    if (currentcon->m_nextConnection == nullptr)
    {
        if (m_currentState->m_stateId != m_noConnection)
            switchCurrentState(owner_, m_noConnection);
        return true;
    }

    auto nextNodePos = nav.m_currentPath->m_graph->getNodePos(currentcon->m_con->m_nodes[currentcon->m_nextNode]);
    auto &ai = owner_.reg->get<ComponentAI>(owner_.idx);
    ai.m_navigationTarget = nextNodePos;
    if (currentcon->m_con->m_traverses[1 - currentcon->m_nextNode] & (1 << Traverse::FallthroughBitID))
        owner_.reg->get<ComponentObstacleFallthrough>(owner_.idx).setIgnoringObstacles();

    auto &traverse = currentcon->m_con->m_traverses[1 - currentcon->m_nextNode];

    if (((traverse >> Traverse::ReservedBits) & static_cast<Traverse::TraitT>(TraverseTraits::FALL)) ||
         ((traverse >> Traverse::ReservedBits) & static_cast<Traverse::TraitT>(TraverseTraits::WALK)))
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
        if (m_currentState->m_stateId != m_noConnection)
            switchCurrentState(owner_, m_noConnection);
    }

    return true;
}

bool JumpTowards::update(EntityAnywhere owner_, uint32_t currentFrame_)
{
    AIState::update(owner_, currentFrame_); 

    auto &ai = owner_.reg->get<ComponentAI>(owner_.idx);
    auto &trans = owner_.reg->get<ComponentTransform>(owner_.idx);

    ai.m_requestedState = m_prejump;
    ai.m_requestedOrientation = ValueToOrientation(ai.m_navigationTarget.x - trans.m_pos.x);

    return true;
}
