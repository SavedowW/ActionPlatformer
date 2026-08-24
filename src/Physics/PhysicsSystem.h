#pragma once
#include "Core/CoreComponents.h"
#include <entt/entt.hpp>
#include <stack>

using CollidersView = decltype(entt::registry{}.view<ComponentStaticCollider>());

enum class AttemptType : uint8_t
{
    // Next attempt can be of any type, but cannot be further than loopback allows. Initial is considered BACKWARD too. Movement is halted on each non-initial BACKWARD attempt
    BACKWARD,

    // RequireMagnet is false for this specific case. Only upward and backward attempts are allowed
    UPWARD,

    // Only downward and backward attempts are allowed
    DOWNWARD
};

struct AttemptPos
{
    using Key = std::tuple<bool, int>;

    AttemptPos(const Vector2<int> &pos_, bool requireMagnet_, bool haltMovement_, AttemptType attemptType_);

    AttemptPos addIgnoredObstacle(entt::entity cid_) const;
    bool isIgnoringObstacle(entt::entity cid_) const;
    const std::set<entt::entity> &ignored() const noexcept;

    Key key() const noexcept;

    const Vector2<int> pos;
    bool requireMagnet;
    bool haltMovement;
    AttemptType attemptType;

private:
    std::set<entt::entity> m_ignored;
};

class AttemptComparatorRight
{
public:
    bool operator()(const AttemptPos::Key &lhs_, const AttemptPos::Key &rhs_) const noexcept;
};

class AttemptComparatorLeft
{
public:
    bool operator()(const AttemptPos::Key &lhs_, const AttemptPos::Key &rhs_) const noexcept;
};

template<typename Cmp>
struct AttemptContainer
{
public:
    // Does need halt and X coordinate
    AttemptContainer();

    void add(AttemptPos &&attempt_);
    AttemptPos extract();
    bool empty() const;

private:
    std::map<AttemptPos::Key, std::stack<AttemptPos>, Cmp> m_attempts;
};

using AttemptContainerRight = AttemptContainer<AttemptComparatorRight>;
using AttemptContainerLeft = AttemptContainer<AttemptComparatorLeft>;

class PhysicsEntityHandler
{
public:
    PhysicsEntityHandler(const CollidersView &cld_, ComponentTransform &trans_, 
        ComponentPhysical &phys_, ComponentObstacleFallthrough &obsFallthrough_, WorldPosition &worldPos_);

    void moveRight(int offset_);
    void moveLeft(int offset_);
    void moveDown(int offset_);
    void moveUp(int offset_);
    
    void magnet();

    /**
     *  Update:
     *   PhysicalEvents
     *    (currently none)
     *   ComponentPhysical:
     *    onSlopeWithAngle
     *    onGround
     *   ComponentObstacleFallthrough:
     *    ignoredObstacles
     */
    void discoverPosition();
    
private:
    const SlopeCollider *getHighestVerticalMagnetCoord(int &coord_);

    const CollidersView &m_cld;

    ComponentTransform &m_trans;
    ComponentPhysical &m_phys;
    const Collider m_pushbox;
    ComponentObstacleFallthrough &m_obsFallthrough;
    WorldPosition &m_worldPos;
    const Vector2<int> m_initialPos;

    bool m_requireMagnet = false;

    static const float VerticalOffsetLimitMul;
};

class PhysicsSystem
{
public:
    PhysicsSystem(entt::registry &reg_);

    void prepHitstop();
    void prepEntities();
    void updatePhysics();

private:
    static void proceedEntity(const CollidersView &clds_, ComponentTransform &trans_, ComponentPhysical &phys_, ComponentObstacleFallthrough &obsFallthrough_, WorldPosition &worldPos_);
    
    entt::registry &m_reg;
    const Vector2<int> m_levelSize;
};
