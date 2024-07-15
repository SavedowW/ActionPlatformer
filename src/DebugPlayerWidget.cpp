#include "DebugPlayerWidget.h"

DebugPlayerWidget::DebugPlayerWidget(Application &application_, Camera &camera_, PlayableCharacter *pc_) :
    m_camera(camera_),
    m_textManager(*application_.getTextManager()),
    m_renderer(*application_.getRenderer()),
    m_pc(pc_),
    m_inputResolver(pc_->m_inputResolver)
{
    auto &texman = *application_.getTextureManager();

    m_arrowIn = texman.getTexture(texman.getTexID("UI/Arrow2"));
    m_arrowOut = texman.getTexture(texman.getTexID("UI/Arrow1"));
}

void DebugPlayerWidget::update()
{
}

void DebugPlayerWidget::draw(Renderer &renderer_, Camera &camera_)
{
    std::string ignoredObstacles = "";
    for (const auto &el : m_pc->m_ignoredObstacles)
        ignoredObstacles += std::to_string(el) + " ";

    std::string cooldowns = "";
    for (const auto &el : m_pc->m_cooldowns)
        cooldowns += std::to_string(!el.isActive());

    m_textManager.renderText("Player pos: " + utils::toString(m_pc->accessPos()), 0, {400, 1});
    m_textManager.renderText("Player vel: " + utils::toString(m_pc->accessPreEditVelocity()), 0, {400, 1 + 22 * 1});
    m_textManager.renderText("Player inr: " + utils::toString(m_pc->m_inertia), 0, {400, 1 + 22 * 2});
    m_textManager.renderText(std::string("Player action: ") + m_pc->getCurrentActionName(), 0, {400, 1 + 22 * 3});
    m_textManager.renderText(std::string("Frames in state: ") + std::to_string(m_pc->m_framesInState), 0, {400, 1 + 22 * 4});
    m_textManager.renderText(std::string("Ignored obstacles: ") + ignoredObstacles, 0, {400, 1 + 22 * 5});
    m_textManager.renderText(cooldowns, 0, {400, 1 + 22 * 6});
    m_textManager.renderText(std::string("On slope: ") + std::to_string(m_pc->m_onSlopeWithAngle), 0, {400, 1 + 22 * 7});

    auto inputs = m_inputResolver.getCurrentInputDir();

    Vector2<float> arrowPos[] = {
        Vector2{850.0f, 20.0f},
        Vector2{885.0f, 55.0f},
        Vector2{850.0f, 90.0f},
        Vector2{815.0f, 55.0f},
    };

    bool isValid[] = {
        inputs.y < 0,
        inputs.x > 0,
        inputs.y > 0,
        inputs.x < 0
    };

    float angles[] = {
        270,
        0,
        90,
        180
    };

    for (int i = 0; i < 4; ++i)
    {
        auto &spr = (isValid[i] ? m_arrowIn : m_arrowOut);
        SDL_FPoint sdlcenter = {spr->m_w / 2, spr->m_h / 2};

        m_renderer.renderTexture(spr->getSprite(),
        arrowPos[i].x, arrowPos[i].y, spr->m_w, spr->m_h, angles[i], &sdlcenter, SDL_FLIP_NONE);
    }
}
