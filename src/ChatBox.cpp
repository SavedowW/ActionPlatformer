#include "ChatBox.h"
#include "utf8.h"

namespace ChatConsts
{
    const int ChatEdgeGap = 4;
    const uint32_t fadeInDuration = 3;
    const uint32_t fadeOutDuration = 5;
}

ChatboxSystem::ChatboxSystem(entt::registry &reg_, Application &app_, Camera &camera_) :
    InputReactor(app_.getInputSystem()),
    m_reg(reg_),
    m_app(app_),
    m_camera(camera_)
{
    subscribe(HUD_EVENTS::PROCEED);
    setInputEnabled();

    auto &texman = *app_.getTextureManager();

    m_chatboxEdge = texman.getTexture(texman.getTexID("UI/chatbox_edge"));
    m_chatboxPointer = texman.getTexture(texman.getTexID("UI/chatbox_pointer"));
}

void ChatboxSystem::setPlayerEntity(entt::entity playerId_)
{
    m_playerId = playerId_;
}

void ChatboxSystem::addSequence(ChatMessageSequence &&seq_)
{
    if (seq_.m_messages.empty())
        throw std::string("ChatboxSystem received an empty sequence");

    m_sequences.emplace_back(std::move(seq_));
    m_sequences.back().compileAndSetSize(*m_app.getTextManager());

    if (m_sequences.size() == 1 && m_sequences[0].m_claimInputs)
    {
        auto &resolver = *m_reg.get<ComponentPlayerInput>(m_playerId).m_inputResolver;
        resolver.setInputDisabled();
        resolver.nullifyCurrentInput();
    }
}

void ChatboxSystem::receiveEvents(HUD_EVENTS event, const float scale_)
{
    switch (event)
    {
        case (HUD_EVENTS::PROCEED):
            if (scale_ > 0)
            {
                for (auto &seq : m_sequences)
                    seq.takeInput();
            }
            break;
    }
}

void ChatboxSystem::renderText(ChatMessageSequence &seq_, const Vector2<int> &tl_)
{
    auto &ren = *m_app.getRenderer();

    auto pos = tl_;
    bool newLine = true;
    int currentLine = 0;
    for (int i = 0; i <= seq_.m_currentMessage->m_currentProceedingCharacter && i < seq_.m_currentMessage->m_symbols.size(); ++i)
    {
        auto &sym = seq_.m_currentMessage->m_symbols[i];
        if (newLine)
        {
            pos.x = tl_.x + seq_.m_currentMessage->m_symbols[0]->m_minx;
            currentLine++;
            newLine = false;
        }
        if (!sym)
        {
            pos.y += seq_.m_currentMessage->m_lineHeights[currentLine];
            newLine = true;
        }
        else if (sym->m_tex)
        {
            bool applyAppearOffset = i >= seq_.m_currentMessage->m_firstCharacterForFadingIn;
            float progress = (applyAppearOffset ?  seq_.m_currentMessage->m_symbolAppearTimers[i].getProgressNormalized() : 1.0f);
            progress = Easing::circ(progress);
            ren.renderTexture(sym->m_tex, pos.x, pos.y - 5 + (5 * progress), sym->m_w, sym->m_h);
            pos.x += sym->m_advance;
        }
    }
}

void ChatboxSystem::update()
{
    bool removeSequence = false;
    if (!m_sequences.empty())
    {
        auto &seq = m_sequences[0];
        if (seq.m_currentState == ChatMessageSequence::BoxState::APPEAR)
        {
            if (seq.m_timer.update())
            {
                seq.m_currentState = ChatMessageSequence::BoxState::IDLE;
                if (!seq.m_currentMessage->m_symbolAppearTimers.empty())
                {
                    seq.m_timer.begin(seq.m_currentMessage->m_defaultCharacterDelay);
                    seq.m_currentMessage->m_symbolAppearTimers[0].begin(seq.m_currentMessage->m_defaultAppearDuration);
                }
            }
        }
        else if (seq.m_currentState == ChatMessageSequence::BoxState::DISAPPEAR)
        {
            if (seq.m_timer.update())
                removeSequence = true;
        }
        else
        {
            // Updating current message
            if (seq.m_currentMessage)
            {
                size_t vsize = seq.m_currentMessage->m_symbolAppearTimers.size();

                if (seq.m_currentMessage->m_currentState == ChatMessage::MessageState::APPEAR)
                {
                    // Iterate over all updating characters, update their timers and boundaries
                    for (size_t i = seq.m_currentMessage->m_firstCharacterForFadingIn; i <= seq.m_currentMessage->m_currentProceedingCharacter && i < vsize; ++i)
                    {
                        if (seq.m_currentMessage->m_symbolAppearTimers[i].update())
                            seq.m_currentMessage->m_firstCharacterForFadingIn++;
                    }

                    // Handle delay between characters
                    if (seq.m_timer.update() && seq.m_currentMessage->m_currentProceedingCharacter < vsize)
                    {
                        seq.m_timer.begin(seq.m_currentMessage->m_defaultCharacterDelay);
                        seq.m_currentMessage->m_currentProceedingCharacter++;

                        if (seq.m_currentMessage->m_currentProceedingCharacter < vsize)
                            seq.m_currentMessage->m_symbolAppearTimers[seq.m_currentMessage->m_currentProceedingCharacter].begin(seq.m_currentMessage->m_defaultAppearDuration);
                    }

                    if (seq.m_currentMessage->m_firstCharacterForFadingIn == seq.m_currentMessage->m_symbolAppearTimers.size())
                        seq.m_currentMessage->m_currentState = ChatMessage::MessageState::IDLE;
                }
            }
        }
    }

    if (removeSequence)
    {
        if (m_sequences[0].m_returnInputs)
        {
            auto &resolver = *m_reg.get<ComponentPlayerInput>(m_playerId).m_inputResolver;
            resolver.setInputEnabled();
            resolver.nullifyCurrentInput();
        }

        m_sequences.erase(m_sequences.begin());

        if (m_sequences.size() > 0 && m_sequences[0].m_claimInputs)
        {
            auto &resolver = *m_reg.get<ComponentPlayerInput>(m_playerId).m_inputResolver;
            resolver.setInputDisabled();
            resolver.nullifyCurrentInput();
        }
    }
}

void ChatboxSystem::draw()
{
    auto &ren = *m_app.getRenderer();
    if (!m_sequences.empty())
        std::cout << "Draw call" << std::endl;
    else
        return;
    auto &seq = m_sequences[0];
    std::cout << "Drawing sequence" << std::endl;
    auto [srcpoint, srctransform] = m_reg.get<HUDPoint, ComponentTransform>(seq.m_source);
    Vector2<float> worldPos = srcpoint.m_pos;
    if (srcpoint.m_posRule == HUDPosRule::REL_TRANSFORM)
        worldPos += srctransform.m_pos;

    bool boxTop = false;
    if (seq.m_side == ChatBoxSide::STRICT_TOP)
    {
        boxTop = true;
    }
    else if (seq.m_side == ChatBoxSide::PREFER_TOP)
    {
        float worldCamPosY = worldPos.y - srcpoint.m_vOffset - m_camera.getTopLeft().y;
        auto camSize = m_camera.getSize();
        int screenPosY = worldCamPosY / camSize.y * gamedata::global::hudLayerResolution.y;

        if (screenPosY >= m_edgeGap + m_chatboxPointer->m_h + seq.m_currentSize.y + m_chatboxEdge->m_h * 2)
            boxTop = true;
    }
    else if (seq.m_side == ChatBoxSide::PREFER_BOTTOM)
    {
        float worldCamPosY = worldPos.y - srcpoint.m_vOffset - m_camera.getTopLeft().y;
        auto camSize = m_camera.getSize();
        int screenPosY = worldCamPosY / camSize.y * gamedata::global::hudLayerResolution.y;

        if (screenPosY > gamedata::global::hudLayerResolution.y - m_edgeGap - m_chatboxPointer->m_h - seq.m_currentSize.y - m_chatboxEdge->m_h * 2)
            boxTop = true;
    }
    else if (seq.m_side == ChatBoxSide::AUTO)
    {
        Vector2<int> screenPos = worldPos - m_camera.getTopLeft();
        auto camSize = m_camera.getSize();
        screenPos.y = screenPos.y / camSize.y * gamedata::global::hudLayerResolution.y;

        boxTop = screenPos.y > (gamedata::global::hudLayerResolution.y - screenPos.y);
    }

    if (boxTop)
        worldPos.y -= srcpoint.m_vOffset;
    else
        worldPos.y += srcpoint.m_vOffset;

    Vector2<int> screenPos = worldPos - m_camera.getTopLeft();
    auto camSize = m_camera.getSize();
    screenPos.x = screenPos.x / camSize.x * gamedata::global::hudLayerResolution.x;
    screenPos.y = screenPos.y / camSize.y * gamedata::global::hudLayerResolution.y;

    ren.fillRectangle(screenPos - Vector2{1, 1}, {2, 2}, {255, 0, 0, 150});

    if (seq.m_fitScreen)
    {
        if (boxTop)
            screenPos.y = utils::clamp(screenPos.y, m_chatboxPointer->m_h + seq.m_currentSize.y + m_chatboxEdge->m_h * 2 + m_edgeGap, gamedata::global::hudLayerResolution.y - m_edgeGap);
        else
            screenPos.y = utils::clamp(screenPos.y, m_edgeGap, gamedata::global::hudLayerResolution.y - m_edgeGap - m_chatboxPointer->m_h - seq.m_currentSize.y + m_chatboxEdge->m_h * 2);

        screenPos.x = utils::clamp(screenPos.x, m_edgeGap + m_chatboxEdge->m_w + m_chatboxPointer->m_w / 2 + 1, gamedata::global::hudLayerResolution.x - m_edgeGap - m_chatboxEdge->m_w - m_chatboxPointer->m_w / 2 - 1);
    }

    ren.fillRectangle(screenPos - Vector2{1, 1}, {2, 2}, {0, 255, 0, 150});

    Vector2<int> iScreenPos = screenPos;

    float progress = seq.m_timer.getProgressNormalized();
    if (seq.m_currentState == ChatMessageSequence::BoxState::IDLE)
        progress = 1.0f;
    else if (seq.m_currentState == ChatMessageSequence::BoxState::DISAPPEAR)
        progress = 1.0f - progress;

    seq.m_currentSize = utils::lerp(seq.m_oldSize, seq.m_targetSize, progress);

    std::cout << seq.m_timer.getCurrentFrame() << std::endl;
    for (auto &el : seq.m_currentMessage->m_symbolAppearTimers)
        std::cout << el.getCurrentFrame() << " ";
    std::cout << std::endl;

    if (seq.m_currentSize.x >= m_chatboxPointer->m_w)
    {
        if (boxTop)
            ren.renderTexture(m_chatboxPointer->getSprite(), screenPos.x - m_chatboxPointer->m_w / 2, screenPos.y - m_chatboxPointer->m_h, m_chatboxPointer->m_w, m_chatboxPointer->m_h, 0, nullptr, SDL_FLIP_VERTICAL);
        else
            ren.renderTexture(m_chatboxPointer->getSprite(), screenPos.x - m_chatboxPointer->m_w / 2, screenPos.y, m_chatboxPointer->m_w, m_chatboxPointer->m_h, 0, nullptr, SDL_FLIP_NONE);
    }

    Vector2<int> outerBoundTL(screenPos.x - seq.m_currentSize.x / 2.0f - m_chatboxEdge->m_w, (boxTop ? screenPos.y - m_chatboxPointer->m_h - seq.m_currentSize.y - m_chatboxEdge->m_h * 2 : screenPos.y + m_chatboxPointer->m_h) + int(boxTop));
    Vector2<int> outerBoundBR(screenPos.x + seq.m_currentSize.x / 2.0f + m_chatboxEdge->m_w, (boxTop ? screenPos.y - m_chatboxPointer->m_h : screenPos.y + m_chatboxPointer->m_h + m_chatboxEdge->m_h * 2 + seq.m_currentSize.y) + int(boxTop));


    if (outerBoundTL.x < m_edgeGap)
    {
        int offset = m_edgeGap - outerBoundTL.x;
        outerBoundTL.x += offset;
        outerBoundBR.x += offset;
    }
    else if (outerBoundBR.x > gamedata::global::hudLayerResolution.x - m_edgeGap)
    {
        int offset = outerBoundBR.x - (gamedata::global::hudLayerResolution.x - m_edgeGap);
        outerBoundTL.x -= offset;
        outerBoundBR.x -= offset;
    }

    //ren.drawRectangle(outerBoundTL, outerBoundBR - outerBoundTL, {255, 0, 0, 255});

    ren.fillRectangle(Vector2<float>(outerBoundTL.x + m_chatboxEdge->m_w, outerBoundTL.y), seq.m_currentSize + Vector2{0.0f, (float)m_chatboxEdge->m_h * 2}, gamedata::colors::LVL4);
    ren.fillRectangle(Vector2<float>(outerBoundTL.x, outerBoundTL.y + m_chatboxEdge->m_h), seq.m_currentSize + Vector2{(float)m_chatboxEdge->m_w * 2, 0.0f}, gamedata::colors::LVL4);

    ren.renderTexture(m_chatboxEdge->getSprite(),
            outerBoundTL.x, outerBoundTL.y,
            m_chatboxEdge->m_w, m_chatboxEdge->m_h, 0, nullptr, SDL_FLIP_NONE);

    ren.renderTexture(m_chatboxEdge->getSprite(),
            outerBoundBR.x - m_chatboxEdge->m_w, outerBoundTL.y,
            m_chatboxEdge->m_w, m_chatboxEdge->m_h, 0, nullptr, SDL_FLIP_HORIZONTAL);

    ren.renderTexture(m_chatboxEdge->getSprite(),
            outerBoundTL.x, outerBoundBR.y - m_chatboxEdge->m_h,
            m_chatboxEdge->m_w, m_chatboxEdge->m_h, 0, nullptr, SDL_FLIP_VERTICAL);

    ren.renderTexture(m_chatboxEdge->getSprite(),
            outerBoundBR.x - m_chatboxEdge->m_w, outerBoundBR.y - m_chatboxEdge->m_h,
            m_chatboxEdge->m_w, m_chatboxEdge->m_h, 0, nullptr, SDL_RendererFlip(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL));

    std::cout << (int)seq.m_currentState << std::endl;
    if (seq.m_currentMessage && seq.m_currentState == ChatMessageSequence::BoxState::IDLE)
        renderText(seq, outerBoundTL + Vector2{m_chatboxEdge->m_w + ChatConsts::ChatEdgeGap, m_chatboxEdge->m_h + ChatConsts::ChatEdgeGap});
}

ChatMessageSequence::ChatMessageSequence(entt::entity src_, const ChatBoxSide &side_, bool fitScreen_, bool proceedByInput_, bool claimInputs_, bool returnInputs_) :
    m_source(src_),
    m_side(side_),
    m_fitScreen(fitScreen_),
    m_proceedByInput(proceedByInput_),
    m_claimInputs(claimInputs_),
    m_returnInputs(returnInputs_)
{
}

void ChatMessageSequence::compileAndSetSize(const TextManager &textMan_)
{
    for (auto &msg : m_messages)
    {
        auto newsz = msg.compileAndGetSize(textMan_);
        m_targetSize.x = std::max(m_targetSize.x, newsz.x);
        m_targetSize.y = std::max(m_targetSize.y, newsz.y);
    }

    m_targetSize.x += ChatConsts::ChatEdgeGap * 2;
    m_targetSize.y += ChatConsts::ChatEdgeGap * 2;

    if (!m_messages.empty())
        m_currentMessage = &m_messages[0];

    m_timer.begin(3);
}

void ChatMessageSequence::takeInput()
{
    if (m_proceedByInput)
    {
        if (m_currentState == BoxState::IDLE && !m_messages.empty())
        {
            if (m_messages[0].m_currentState == ChatMessage::MessageState::APPEAR)
                m_messages[0].skip();
            else if (m_messages[0].m_currentState == ChatMessage::MessageState::IDLE)
                m_messages.erase(m_messages.begin());
            if (m_messages.empty())
            {
                m_currentState = BoxState::DISAPPEAR;
                m_timer.begin(ChatConsts::fadeOutDuration);
            }
            else
            {
                m_currentMessage = &m_messages[0];
            }
        }
    }
}

Vector2<int> ChatMessage::compileAndGetSize(const TextManager &textMan_)
{
    Vector2<int> m_size;
    Vector2<int> m_currentLineSize;

    U8Wrapper wrp(m_textRaw);

    bool newLine = true;

    for (auto &ch : wrp)
    {
        auto sym = textMan_.getSymbol(m_baseFont, ch.getu8());

        if (newLine)
        {
            m_currentLineSize.x = sym->m_minx;
            m_currentLineSize.y = textMan_.getFontHeight(m_baseFont);
            newLine = false;
        }

        if (*ch.m_ch == '\n')
        {
            m_size.y += m_currentLineSize.y;
            m_size.x = std::max(m_size.x, m_currentLineSize.x);
            m_lineHeights.push_back(m_currentLineSize.y);

            m_currentLineSize.x = 0;
            m_currentLineSize.y = 0;

            newLine = true;

            m_symbols.push_back(nullptr);
        }
        else
        {
            m_symbols.push_back(sym);
        }

        m_currentLineSize.x += sym->m_advance;
    }

    m_size.y += m_currentLineSize.y;
    m_size.x = std::max(m_size.x, m_currentLineSize.x);
    m_lineHeights.push_back(m_currentLineSize.y);

    m_symbolAppearTimers.resize(m_symbols.size());
    m_symbolAppearTimers[0].begin(m_defaultAppearDuration);

    return m_size;
}

void ChatMessage::skip()
{
    m_currentState = MessageState::IDLE;

    for (auto &el : m_symbolAppearTimers)
        el.forceOver();
    m_currentProceedingCharacter = m_symbolAppearTimers.size();
    m_firstCharacterForFadingIn = m_symbolAppearTimers.size();
}
