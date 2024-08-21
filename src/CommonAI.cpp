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

    auto &nav = owner_.reg->get<Navigatable>(owner_.idx);

    if (!nav.m_currentPath)
    {
        nav.m_currentPath = owner_.reg->get<World>(m_target.idx).getNavsys().makePath(nav.m_traverseTraits, m_target.idx);
    }

    if (nav.m_currentOwnConnection)
        nav.m_currentPath->buildUntil(nav.m_currentOwnConnection);

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
