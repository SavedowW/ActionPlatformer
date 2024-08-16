#include <iostream>
#include <string>

#include "Stage1.h"
#include "Application.h"
#include "TimelineProperty.h"

int main(int argc, char* args[])
{    
    Application app;

    NavGraph m_graph(app);

    auto nd1 = m_graph.makeNode(Vector2{0, 25}.mulComponents(gamedata::global::tileSize) + Vector2{0.0f, gamedata::global::tileSize.y / 2.0f});
    auto nd2 = m_graph.makeNode(Vector2{6, 25}.mulComponents(gamedata::global::tileSize) + Vector2{0.0f, gamedata::global::tileSize.y / 2.0f});
    auto nd3 = m_graph.makeNode(Vector2{10, 21}.mulComponents(gamedata::global::tileSize) + Vector2{0.0f, gamedata::global::tileSize.y / 2.0f});
    auto nd4 = m_graph.makeNode(Vector2{21, 21}.mulComponents(gamedata::global::tileSize) + Vector2{0.0f, gamedata::global::tileSize.y / 2.0f});
    auto nd5 = m_graph.makeNode(Vector2{25, 25}.mulComponents(gamedata::global::tileSize) + Vector2{0.0f, gamedata::global::tileSize.y / 2.0f});
    auto nd6 = m_graph.makeNode(Vector2{22, 27}.mulComponents(gamedata::global::tileSize) + Vector2{0.0f, gamedata::global::tileSize.y / 2.0f});
    auto nd7 = m_graph.makeNode(Vector2{21, 27}.mulComponents(gamedata::global::tileSize) + Vector2{0.0f, gamedata::global::tileSize.y / 2.0f});
    auto nd8 = m_graph.makeNode(Vector2{19, 29}.mulComponents(gamedata::global::tileSize) + Vector2{0.0f, gamedata::global::tileSize.y / 2.0f});
    auto nd9 = m_graph.makeNode(Vector2{10, 29}.mulComponents(gamedata::global::tileSize) + Vector2{0.0f, gamedata::global::tileSize.y / 2.0f});

    auto nd10 = m_graph.makeNode(Vector2{29, 25}.mulComponents(gamedata::global::tileSize) + Vector2{0.0f, gamedata::global::tileSize.y / 2.0f});
    auto nd11 = m_graph.makeNode(Vector2{33, 29}.mulComponents(gamedata::global::tileSize) + Vector2{0.0f, gamedata::global::tileSize.y / 2.0f});
    auto nd12 = m_graph.makeNode(Vector2{35, 29}.mulComponents(gamedata::global::tileSize) + Vector2{0.0f, gamedata::global::tileSize.y / 2.0f});
    auto nd13 = m_graph.makeNode(Vector2{36, 25}.mulComponents(gamedata::global::tileSize) + Vector2{0.0f, gamedata::global::tileSize.y / 2.0f});
    auto nd14 = m_graph.makeNode(Vector2{37, 25}.mulComponents(gamedata::global::tileSize) + Vector2{0.0f, gamedata::global::tileSize.y / 2.0f});
    auto nd15 = m_graph.makeNode(Vector2{39, 22}.mulComponents(gamedata::global::tileSize) + Vector2{0.0f, gamedata::global::tileSize.y / 2.0f});
    auto nd16 = m_graph.makeNode(Vector2{40, 22}.mulComponents(gamedata::global::tileSize) + Vector2{0.0f, gamedata::global::tileSize.y / 2.0f});
    auto nd17 = m_graph.makeNode(Vector2{40, 19}.mulComponents(gamedata::global::tileSize) + Vector2{0.0f, gamedata::global::tileSize.y / 2.0f});
    auto nd18 = m_graph.makeNode(Vector2{41, 19}.mulComponents(gamedata::global::tileSize) + Vector2{0.0f, gamedata::global::tileSize.y / 2.0f});
    auto nd19 = m_graph.makeNode(Vector2{42, 19}.mulComponents(gamedata::global::tileSize) + Vector2{0.0f, gamedata::global::tileSize.y / 2.0f});
    auto nd20 = m_graph.makeNode(Vector2{47, 19}.mulComponents(gamedata::global::tileSize) + Vector2{0.0f, gamedata::global::tileSize.y / 2.0f});

    auto nd21 = m_graph.makeNode(Vector2{38, 29}.mulComponents(gamedata::global::tileSize) + Vector2{0.0f, gamedata::global::tileSize.y / 2.0f});
    auto nd22 = m_graph.makeNode(Vector2{49, 29}.mulComponents(gamedata::global::tileSize) + Vector2{0.0f, gamedata::global::tileSize.y / 2.0f});
    auto nd23 = m_graph.makeNode(Vector2{53, 29}.mulComponents(gamedata::global::tileSize) + Vector2{0.0f, gamedata::global::tileSize.y / 2.0f});

    m_graph.makeConnection(nd1, nd2, 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP), 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP));

    m_graph.makeConnection(nd2, nd3, 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP),
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP));

    m_graph.makeConnection(nd3, nd4, 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP), 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP));

    m_graph.makeConnection(nd4, nd5, 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP), 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP));

    m_graph.makeConnection(nd2, nd9, 
        Traverse::makeSignature(true, TraverseTraits::WALK, TraverseTraits::JUMP), 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP));

    m_graph.makeConnection(nd8, nd9, 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP), 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP));

    m_graph.makeConnection(nd7, nd8, 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP), 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP));

    m_graph.makeConnection(nd6, nd7, 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP),
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP));

    m_graph.makeConnection(nd5, nd6, 
        Traverse::makeSignature(true, TraverseTraits::WALK, TraverseTraits::JUMP), 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP));

    m_graph.makeConnection(nd5, nd10, 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP), 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP));

    m_graph.makeConnection(nd10, nd11, 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP), 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP));

    m_graph.makeConnection(nd11, nd12, 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP), 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP));

    m_graph.makeConnection(nd12, nd13, 
        Traverse::makeSignature(false, TraverseTraits::JUMP), 
        Traverse::makeSignature(false, TraverseTraits::FALL));

    m_graph.makeConnection(nd13, nd14, 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP), 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP));

    m_graph.makeConnection(nd14, nd21,
        Traverse::makeSignature(false, TraverseTraits::FALL), 
        Traverse::makeSignature(false, TraverseTraits::JUMP));

    m_graph.makeConnection(nd21, nd22,
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP), 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP));

    m_graph.makeConnection(nd22, nd23,
        Traverse::makeSignature(false, TraverseTraits::WALK), 
        Traverse::makeSignature(false, TraverseTraits::WALK));

    m_graph.makeConnection(nd14, nd15,
        Traverse::makeSignature(false, TraverseTraits::JUMP), 
        Traverse::makeSignature(false, TraverseTraits::JUMP));

    m_graph.makeConnection(nd15, nd16,
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP), 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP));

    m_graph.makeConnection(nd15, nd17,
        Traverse::makeSignature(false, TraverseTraits::JUMP), 
        Traverse::makeSignature(false, TraverseTraits::FALL));

    m_graph.makeConnection(nd17, nd18,
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP), 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP));

    m_graph.makeConnection(nd18, nd19,
        Traverse::makeSignature(false, TraverseTraits::JUMP), 
        Traverse::makeSignature(false, TraverseTraits::JUMP));

    m_graph.makeConnection(nd19, nd20,
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP), 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP));

    m_graph.makeConnection(nd20, nd22,
        Traverse::makeSignature(false, TraverseTraits::FALL), 
        Traverse::makeSignature(false));

    NavPath path(&m_graph, entt::null, Traverse::makeSignature(true, TraverseTraits::WALK, TraverseTraits::JUMP,  TraverseTraits::FALL));
    path.m_currentTarget = 22;

    std::cout << std::boolalpha << path.buildUntil(path.m_fullGraph[0].m_con) << std::endl;
    path.dump();

    std::cout << app.getBasePath() << std::endl;
    //RecipeParser pr(app.getAnimationManager(), app.getBasePath() + "/Resources/data.json");

    app.setLevel(1, new Stage1(&app, {2048, 2048}, 1));
    app.run();

    return 0;
}