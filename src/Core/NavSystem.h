#ifndef NAV_SYSTEM_H_
#define NAV_SYSTEM_H_
#include "Application.h"
#include "NavGraph.h"
#include "Camera.h"
#include <entt/entt.hpp>

struct NavSystem;

// A view to a selected connection for pathfinding algo
struct ConnectionDescr
{
    enum class Status
    {
        UNRESOLVED,
        FOUND,
        NOT_EXISTS
    };

    // TODO: make oriented
    ConnectionDescr(const Connection &con_);

    const Connection &m_originalCon;

    // Actual total cost until target
    float m_calculatedCost;
    
    std::vector<ConnectionDescr *> m_neighbourConnections;
    std::optional<const ConnectionDescr *> m_nextConnection;
    uint8_t m_nextNode = 0;

    // If next connection is known, return begin, end
    std::pair<NodeID, NodeID> getOrientedNodes() const;

    void resetResults();
    void setPathFound(const ConnectionDescr *con_, float calculatedCost_, uint8_t nextNode_);
    void setNoPathFound();

    Status getStatus() const;
};

class NavPath
{
public:
    enum class Status
    {
        FOUND,
        NOT_FOUND,
        FINISHED // Source and destination have the same connection ID
    };

    struct Follower
    {
        std::shared_ptr<NavPath> m_path;
        const Connection *m_currentOwnConnection = nullptr;

        Vector2<float> getNextNodePos() const;
        bool nextConnectionExists() const;
        void iterateForward();
        std::pair<Vector2<float>, Vector2<float>> getCurrentNodes() const;
    };

    NavPath(const NavGraph &graph_, entt::entity target_, entt::registry &reg_, Traverse::TraitT traits_, float targetMaxConnectionRange_);

    // TODO: by connection ID
    Status buildUntil(const Connection * const con_);

    /*
        Read current connection of the target
        If it's changed - clear current costs and next connections
    */
    void update();
    void dump() const;
    void updateTarget();

    bool isTargetConnection(ConnectionID id_) const;
    const Connection *findClosestConnection(const Vector2<float> &pos_, Traverse::TraitT traits_, float maxRange_) const;
    
    // TODO: unordered_multimap
    std::unordered_map<ConnectionID, ConnectionDescr> m_graphView;
    const NavGraph &m_graph;
    
    private:

    const Traverse::TraitT m_traverseTraits;
    const entt::entity m_target;
    
    // Max range to be tied to a connection
    const float m_targetMaxConnectionRange;
    
    std::vector<ConnectionDescr *> m_front;
    const Connection *m_currentTarget = nullptr;
    entt::registry &m_reg;

    friend struct NavSystem;    
};

/*
    System, responsible for path management
    Each path is essentially a resource, identified by it's traverse traits and target entity
        (TODO, currently only traits)
    It's users share the same path and reuse constructed parts
*/
struct NavSystem
{
    NavSystem(entt::registry &reg_, Application &app_, NavGraph &graph_);

    void update();
    void draw(Camera &cam_);

    // Get existing path instance or create new
    NavPath::Follower makePath(Traverse::TraitT traverseTraits_, entt::entity goal_, float maxTarRange_);

    entt::registry &m_reg;
    Renderer &m_ren;
    TextManager &m_textman;
    NavGraph &m_graph;

    std::unordered_map<Traverse::TraitT, std::weak_ptr<NavPath>> m_paths;
};

#endif
