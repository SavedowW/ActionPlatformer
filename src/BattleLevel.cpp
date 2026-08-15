#include "BattleLevel.h"
#include "Core/FilesystemUtils.h"
#include "SM/StateMachine.hpp"
#include "Core/GameData.h"
#include "Core/Application.h"
#include "Core/Localization/LocalizationGen.h"
#include "Core/Profile.h"
#include <fstream>

Vector2<int> extractSize(std::string filename_)
{
    const auto fullpath = Filesystem::getRootDirectory() + filename_;
    
    std::ifstream mapjson(fullpath);
    if (!mapjson.is_open())
        throw std::runtime_error(std::format("Failed to open map description at \"{}\"", fullpath));

    nlohmann::json mapdata = nlohmann::json::parse(mapjson);

    return {
        mapdata.at("width").get<int>() * mapdata.at("tilewidth").get<int>(),
        mapdata.at("height").get<int>() * mapdata.at("tileheight").get<int>()
    };
}

BattleLevel::BattleLevel(FPSUtility &fpsUtility_, std::string filename_) :
    Level(std::filesystem::path(filename_).filename().replace_extension().string(), fpsUtility_, extractSize(filename_)),
    m_fileName{std::move(filename_)},
    m_camera({0, 0}, gamedata::global::maxCameraSize, m_size),
    m_playerSystem(m_registry, m_partsys, m_camera),
    m_rendersys(m_registry, m_camera, m_cldRoutesCollection),
    m_inputsys(m_registry),
    m_physsys(m_registry),
    m_camsys(m_registry, m_camera, m_playerSystem),
    m_hudsys(m_registry, m_camera, m_levelName, m_size, m_playerSystem),
    m_enemysys(m_registry, m_navsys, m_camera, m_partsys, m_playerSystem),
    m_aisys(m_registry),
    m_navsys(m_registry, m_graph),
    m_colsys(m_registry),
    m_partsys(m_registry),
    m_battlesys(m_registry, m_camera),
    m_chatBoxSys(m_registry, m_camera),
    m_envSystem(m_registry),
    m_lvlBuilder(m_registry)
{
    //m_envSystem.makeGrassTop(Vector2{230, 351});
        
    subscribe(GAMEPLAY_EVENTS::FN4);

//for (int i = 0; i < 1000; ++i)
    //m_enemyId = m_enemysys.makeEnemy();
}

void BattleLevel::enter()
{
    Level::enter();

    m_lvlBuilder.buildLevel(m_fileName, m_graph, m_cldRoutesCollection);
    m_playerSystem.createPlayer();

    m_camera.setScale(1.0f);
    m_camera.setPos({320, 383});
}

void BattleLevel::receiveEvents(GAMEPLAY_EVENTS event, const float scale_)
{
    switch (event)
    {
        case GAMEPLAY_EVENTS::FN4:
            if (scale_ > 0)
            {
                ChatMessageSequence seq{m_playerSystem.getPlayerId(), ChatBoxSide::PREFER_TOP, true};
                seq.addMessage(ll::test_dlg1());
                seq.addMessage(ll::test_dlg2());
                seq.addMessage(ll::test_dlg3());
                m_chatBoxSys.addSequence(std::move(seq));

                #if 0
                ChatMessageSequence seq2{entt::null, ChatBoxSide::PREFER_BOTTOM, true, true, true, true};
                seq2.addMessage({ll::test_dlg4(), 3});
                seq2.addMessage({ll::test_dlg5(), 3});
                m_chatBoxSys.addSequence(std::move(seq2));
                #endif
            }
            else
                Level::receiveEvents(event, scale_);
        break;

        default:
            Level::receiveEvents(event, scale_);
    }

}

void BattleLevel::update()
{
    PROFILE_FUNCTION;

    m_physsys.prepHitstop();

    m_inputsys.update();

    m_rendersys.update();

    m_navsys.update();

    m_aisys.update();

    m_playerSystem.update();

    m_physsys.prepEntities();

    m_colsys.updateMovingColliders();

    m_partsys.update();

    m_physsys.updatePhysics();
    
    m_battlesys.update();
    m_battlesys.handleAttacks();

    m_envSystem.update();

    m_camsys.update();

    m_chatBoxSys.update();

    /*
        Just updates camera shake logic, but many systems can cause shake
    */
    m_camera.update();

    m_rendersys.updateDepth();
}

void BattleLevel::draw() const
{
    PROFILE_FUNCTION;

    auto &renderer = Application::instance().m_renderer;
    renderer.switchToWorld(gamedata::colors::LVL2);

    m_rendersys.draw();

    m_graph.draw(m_camera);
    m_navsys.draw(m_camera);

    m_camsys.debugDraw(renderer, m_camera);

    m_battlesys.debugDraw();

    renderer.switchToHUD({0, 0, 0, 0});

    m_chatBoxSys.draw();

    renderer.switchToDBG({0, 0, 0, 0});

    m_hudsys.draw();

    renderer.updateScreen(m_camera);
}
