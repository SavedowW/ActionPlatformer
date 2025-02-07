#include "ChatBox.h"
#include "utf8.h"

namespace ChatConsts
{
    const int ChatEdgeGap = 4;
}

ChatboxSystem::ChatboxSystem(entt::registry &reg_, Application &app_, Camera &camera_) :
    InputReactor(app_.getInputSystem()),
    m_reg(reg_),
    m_app(app_),
    m_camera(camera_)
{
    subscribe(EVENTS::ATTACK);
    setInputEnabled(true);

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

    if (m_sequences.back().m_claimInputs)
    {
        auto &resolver = *m_reg.get<ComponentPlayerInput>(m_playerId).m_inputResolver;
        resolver.setInputEnabled(false);
        resolver.nullifyCurrentInput();
    }
}

void ChatboxSystem::receiveInput(EVENTS event, const float scale_)
{
    std::cout << "EVENT\n";
    switch (event)
    {
        case (EVENTS::ATTACK):
            if (scale_ > 0)
            {
                int seqid = 0;
                while (seqid < m_sequences.size())
                {
                    auto &seq = m_sequences[seqid];
                    if (seq.m_proceedByInput)
                    {
                        seq.m_messages.erase(seq.m_messages.begin());
                        if (seq.m_messages.empty())
                        {
                            endSequence(seqid);
                        }
                        else
                        {
                            seq.m_currentMessage = &seq.m_messages[0];
                            seqid++;
                        }
                    }
                    else
                        seqid++;
                }
            }
            break;
    }
}

void ChatboxSystem::endSequence(size_t seqId_)
{
    if (m_sequences[seqId_].m_returnInputs)
    {
        auto &resolver = *m_reg.get<ComponentPlayerInput>(m_playerId).m_inputResolver;
        resolver.setInputEnabled(true);
        resolver.nullifyCurrentInput();
    }

    m_sequences.erase(m_sequences.begin() + seqId_);
}

void ChatboxSystem::renderText(ChatMessageSequence &seq_, const Vector2<float> &tl_)
{
    auto &ren = *m_app.getRenderer();

    auto pos = tl_;
    bool newLine = true;
    int currentLine = 0;
    for (int i = 0; i < seq_.m_currentMessage->m_currentProceedingCharacter && i < seq_.m_currentMessage->m_symbols.size(); ++i)
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
            ren.renderTexture(sym->m_tex, pos.x, pos.y - 5.0f + (5.0f * progress), sym->m_w, sym->m_h);
            pos.x += sym->m_advance;
        }
    }
}

void ChatboxSystem::draw()
{
    auto &ren = *m_app.getRenderer();
    if (!m_sequences.empty())
        std::cout << "Draw call" << std::endl;
    for (auto &seq : m_sequences)
    {
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

        ren.fillRectangle(screenPos - Vector2{1.0f, 1.0f}, {2.0f, 2.0f}, {255, 0, 0, 150});

        if (seq.m_fitScreen)
        {
            if (boxTop)
                screenPos.y = utils::clamp(screenPos.y, m_chatboxPointer->m_h + seq.m_currentSize.y + m_chatboxEdge->m_h * 2 + m_edgeGap, gamedata::global::hudLayerResolution.y - m_edgeGap);
            else
                screenPos.y = utils::clamp(screenPos.y, m_edgeGap, gamedata::global::hudLayerResolution.y - m_edgeGap - m_chatboxPointer->m_h - seq.m_currentSize.y + m_chatboxEdge->m_h * 2);

            screenPos.x = utils::clamp(screenPos.x, m_edgeGap + m_chatboxEdge->m_w + m_chatboxPointer->m_w / 2 + 1, gamedata::global::hudLayerResolution.x - m_edgeGap - m_chatboxEdge->m_w - m_chatboxPointer->m_w / 2 - 1);
        }

        ren.fillRectangle(screenPos - Vector2{1.0f, 1.0f}, {2.0f, 2.0f}, {0, 255, 0, 150});

        Vector2<int> iScreenPos = screenPos;

        seq.m_timer.update();
        seq.m_currentSize = utils::lerp(seq.m_oldSize, seq.m_targetSize, seq.m_timer.getProgressNormalized());

        if (seq.m_timer.isOver() && seq.m_currentMessage->m_currentProceedingCharacter < seq.m_currentMessage->m_symbols.size())
        {
            if (seq.m_characterTimer.update())
            {
                seq.m_currentMessage->m_symbolAppearTimers[seq.m_currentMessage->m_currentProceedingCharacter].begin(seq.m_currentMessage->m_defaultAppearDuration);

                seq.m_currentMessage->m_currentProceedingCharacter++;

                if (seq.m_currentMessage->m_currentProceedingCharacter < seq.m_currentMessage->m_symbols.size())
                    seq.m_characterTimer.begin(seq.m_currentMessage->m_defaultCharacterDelay);
            }
            
        }

        if (seq.m_timer.isOver() && seq.m_currentMessage->m_currentState == ChatMessage::State::APPEAR)
        {
            for (int i = seq.m_currentMessage->m_firstCharacterForFadingIn; i <= seq.m_currentMessage->m_currentProceedingCharacter && i < seq.m_currentMessage->m_symbols.size(); ++i)
            {
                if (seq.m_currentMessage->m_symbolAppearTimers[i].update())
                    if (i == seq.m_currentMessage->m_firstCharacterForFadingIn)
                        seq.m_currentMessage->m_firstCharacterForFadingIn++;
            }

            if (seq.m_currentMessage->m_firstCharacterForFadingIn == seq.m_currentMessage->m_currentProceedingCharacter && seq.m_currentMessage->m_currentProceedingCharacter >= seq.m_currentMessage->m_symbols.size())
                seq.m_currentMessage->m_currentState = ChatMessage::State::IDLE;
        }

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

        if (seq.m_currentMessage && !seq.m_timer.isActive())
            renderText(seq, outerBoundTL + Vector2{m_chatboxEdge->m_w + ChatConsts::ChatEdgeGap, m_chatboxEdge->m_h + ChatConsts::ChatEdgeGap});
    }
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
