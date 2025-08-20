#include "Core/Application.h"
#include "Core/CoreComponents.h"
#include "Core/GameData.h"
#include "Core/InputResolver.h"
#include "Core/utf8.h"
#include "ChatBox.h"

uint32_t ChatMessage::m_defaultCharacterDelay = 2;
uint32_t ChatMessage::m_defaultAppearDuration = 12;

template <>
inline std::string InlinedValueHandler::getParam<std::string>(size_t index_)
{
    return m_tokens[index_];
}

template <>
inline int InlinedValueHandler::getParam<int>(size_t index_)
{
    return std::atoi(m_tokens[index_].c_str());
}

template <>
inline std::string InlinedValueHandler::getParam<std::string>(size_t index_, const std::string &default_)
{
    if (index_ >= m_tokens.size() || m_tokens[index_] == "default")
        return default_;

    return m_tokens[index_];
}

template <>
inline int InlinedValueHandler::getParam<int>(size_t index_, const int &default_)
{
    if (index_ >= m_tokens.size() || m_tokens[index_] == "default")
        return default_;

    return std::atoi(m_tokens[index_].c_str());
}

template <>
inline uint32_t InlinedValueHandler::getParam<uint32_t>(size_t index_, const uint32_t &default_)
{
    if (index_ >= m_tokens.size() || m_tokens[index_] == "default")
        return default_;

    return std::atoi(m_tokens[index_].c_str());
}

template <>
inline float InlinedValueHandler::getParam<float>(size_t index_, const float &default_)
{
    if (index_ >= m_tokens.size() || m_tokens[index_] == "default")
        return default_;

    return std::stof(m_tokens[index_].c_str());
}

std::unique_ptr<fonts::Symbol> processCommand(const std::string &cmd_)
{
    auto eqpos = cmd_.find_first_of('=');
    auto cmd = eqpos != std::string::npos ? cmd_.substr(0, eqpos) : cmd_;
    InlinedValueHandler val(eqpos != std::string::npos ? cmd_.substr(eqpos + 1) : "");

    if (cmd == "delay")
        return std::unique_ptr<fonts::Symbol>(new SymbolDelay(val.getParam<int>(0)));
    else if (cmd == "charspd")
    {
        return std::unique_ptr<fonts::Symbol>(new SymbolSetCharacterSpeed(
            val.getParam(0, ChatMessage::m_defaultCharacterDelay),
            val.getParam(1, ChatMessage::m_defaultAppearDuration)
        ));
    }
    else if (cmd == "shake")
    {
        return std::unique_ptr<fonts::Symbol>(new SymbolRenderShake(
            val.getParam(0, 0),
            val.getParam(1, 0),
            val.getParam(2, 1.0f)
        ));
    }

    else if (cmd == "/shake")
    {
        return std::unique_ptr<fonts::Symbol>(new RenderDropSymbol<SymbolRenderShake>);
    }

    return nullptr;
}

namespace ChatConsts
{
    const int ChatEdgeGap = 4;
    const uint32_t fadeInDuration = 5;
    const uint32_t fadeBetweenDuration = 5;
    const uint32_t fadeOutDuration = 7;
}

ChatboxSystem::ChatboxSystem(entt::registry &reg_, Camera &camera_) :
    m_reg(reg_),
    m_camera(camera_),
    m_renderer(Application::instance().m_renderer)
{
    subscribe(HUD_EVENTS::PROCEED);
    setInputEnabled();

    auto &texman = Application::instance().m_textureManager;

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
        throw std::runtime_error("ChatboxSystem received an empty sequence");

    m_sequences.emplace_back(std::move(seq_));
    m_sequences.back().compileAndSetSize();

    if (m_sequences.size() == 1 && m_sequences[0].m_claimInputs)
    {
        auto &resolver = m_reg.get<InputResolver>(m_playerId);
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

        default:
        break;
    }
}

template<typename T>
void clearStack(std::stack<T> &stack_)
{
    stack_ = std::stack<T>();
}

void ChatboxSystem::renderText(ChatMessageSequence &seq_, const Vector2<int> &tl_)
{
    std::apply([](auto&... ptrs) { ((clearStack(ptrs)), ...); }, seq_.m_renderEffects);

    auto pos = tl_;
    bool newLine = true;
    int currentLine = 0;
    for (size_t i = 0; i <= seq_.m_currentMessage->m_currentProceedingCharacter && i < seq_.m_currentMessage->m_symbols.size(); ++i)
    {
        auto &sym = seq_.m_currentMessage->m_symbols[i];
        if (newLine)
        {
            pos.x = tl_.x - seq_.m_currentMessage->m_symbols[0]->m_minx;
            currentLine++;
            newLine = false;
        }
        if (!sym)
        {
            pos.y += seq_.m_currentMessage->m_lineHeights[currentLine];
            newLine = true;
        }
        else if (sym->m_tex.m_id)
        {
            const bool applyAppearOffset = i >= seq_.m_currentMessage->m_firstCharacterForFadingIn;
            const float progress = applyAppearOffset ? Easing::circ(seq_.m_currentMessage->m_symbolAppearTimers[i].getProgressNormalized()) : 1.0f;

            Vector2<int> offset;
            {
                auto &stack = std::get<std::stack<SymbolRenderShake*>>(seq_.m_renderEffects);
                if (!stack.empty())
                    offset = stack.top()->getOffset();
            }

            //ren.drawRectangle({pos.x, pos.y - 5 + int(5 * progress)}, sym->m_tex.m_size, SDL_Color{255, 255, 255, 255});
            m_renderer.renderTexture(sym->m_tex.m_id, Vector2{pos.x, pos.y - 5 + int(5 * progress)} + offset, sym->m_tex.m_size, SDL_FLIP_NONE, progress);
            pos.x += sym->m_advance;
        }
        else if (auto renSym = const_cast<IRenderSymbol*>(dynamic_cast<const IRenderSymbol*>(sym)))
        {
            renSym->onRenderReached(seq_);
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
            if (seq.m_windowTimer.update())
            {
                seq.m_currentState = ChatMessageSequence::BoxState::IDLE;
                if (!seq.m_currentMessage->m_symbolAppearTimers.empty())
                {
                    seq.m_currentMessage->m_charDelayTimer.begin(seq.m_currentMessage->m_characterDelay);
                    seq.m_currentMessage->m_symbolAppearTimers[0].begin(seq.m_currentMessage->m_appearDuration);
                }
            }
        }
        else if (seq.m_currentState == ChatMessageSequence::BoxState::DISAPPEAR)
        {
            if (seq.m_windowTimer.update())
                removeSequence = true;
        }
        else // Proceeding message
        {
            seq.m_windowTimer.update();
            // Updating current message
            if (seq.m_currentMessage)
            {
                // Full amount of characters in the current message (or specifically number of character-specific timers)
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
                    if (seq.m_currentMessage->m_charDelayTimer.update() && seq.m_currentMessage->m_currentProceedingCharacter < vsize)
                    {
                        seq.m_currentMessage->proceedUntilNonTechCharacter();

                        seq.m_currentMessage->m_charDelayTimer.begin(seq.m_currentMessage->m_characterDelay);
                        
                        if (seq.m_currentMessage->m_currentProceedingCharacter < vsize)
                            seq.m_currentMessage->m_symbolAppearTimers[seq.m_currentMessage->m_currentProceedingCharacter].begin(seq.m_currentMessage->m_appearDuration);
                    }

                    if (seq.m_currentMessage->m_firstCharacterForFadingIn >= seq.m_currentMessage->m_symbolAppearTimers.size())
                        seq.m_currentMessage->m_currentState = ChatMessage::MessageState::IDLE;
                }
            }
        }
    }

    if (removeSequence)
    {
        if (m_sequences[0].m_returnInputs)
        {
            auto &resolver = m_reg.get<InputResolver>(m_playerId);
            resolver.setInputEnabled();
            resolver.nullifyCurrentInput();
        }

        m_sequences.erase(m_sequences.begin());

        if (m_sequences.size() > 0 && m_sequences[0].m_claimInputs)
        {
            auto &resolver = m_reg.get<InputResolver>(m_playerId);
            resolver.setInputDisabled();
            resolver.nullifyCurrentInput();
        }
    }
}

void ChatboxSystem::draw()
{
    if (!m_sequences.empty())
        std::cout << "Draw call" << std::endl;
    else
        return;
    auto &seq = m_sequences[0];
    std::cout << "Drawing sequence" << std::endl;
    const auto &srcpoint = m_reg.get<HUDPoint>(seq.m_source);
    auto worldPos = srcpoint.m_pos;
    if (srcpoint.m_posRule == HUDPosRule::REL_TRANSFORM)
    {
        worldPos += m_reg.get<ComponentTransform>(seq.m_source).m_pos;
    }

    const auto camSize = m_camera.getSize();

    bool boxTop = false;
    if (seq.m_side == ChatBoxSide::STRICT_TOP)
    {
        boxTop = true;
    }
    else if (seq.m_side == ChatBoxSide::PREFER_TOP)
    {
        const auto worldCamPosY = worldPos.y - srcpoint.m_vOffset - m_camera.getTopLeft().y;
        const auto screenPosY = worldCamPosY / camSize.y * gamedata::global::hudLayerResolution.y;

        if (screenPosY >= m_edgeGap + m_chatboxPointer->m_size.y + seq.m_currentSize.y + m_chatboxEdge->m_size.y * 2)
            boxTop = true;
    }
    else if (seq.m_side == ChatBoxSide::PREFER_BOTTOM)
    {
        const auto worldCamPosY = worldPos.y - srcpoint.m_vOffset - m_camera.getTopLeft().y;
        const auto screenPosY = worldCamPosY / camSize.y * gamedata::global::hudLayerResolution.y;

        if (screenPosY > gamedata::global::hudLayerResolution.y - m_edgeGap - m_chatboxPointer->m_size.y - seq.m_currentSize.y - m_chatboxEdge->m_size.y * 2)
            boxTop = true;
    }
    else if (seq.m_side == ChatBoxSide::AUTO)
    {
        auto screenPos = worldPos - m_camera.getTopLeft();
        screenPos.y = screenPos.y / camSize.y * gamedata::global::hudLayerResolution.y;

        boxTop = screenPos.y > (gamedata::global::hudLayerResolution.y - screenPos.y);
    }

    if (boxTop)
        worldPos.y -= srcpoint.m_vOffset;
    else
        worldPos.y += srcpoint.m_vOffset;

    Vector2<float> screenPos = worldPos - m_camera.getTopLeft();
    screenPos.x = screenPos.x / camSize.x * gamedata::global::hudLayerResolution.x;
    screenPos.y = screenPos.y / camSize.y * gamedata::global::hudLayerResolution.y;

    m_renderer.fillRectangle(screenPos - Vector2{1, 1}, {2, 2}, {255, 0, 0, 150});

    if (seq.m_fitScreen)
    {
        if (boxTop)
            screenPos.y = utils::clamp(screenPos.y, static_cast<float>(m_chatboxPointer->m_size.y + seq.m_currentSize.y + m_chatboxEdge->m_size.y * 2 + m_edgeGap), static_cast<float>(gamedata::global::hudLayerResolution.y - m_edgeGap));
        else
            screenPos.y = utils::clamp(screenPos.y, static_cast<float>(m_edgeGap), static_cast<float>(gamedata::global::hudLayerResolution.y - m_edgeGap - m_chatboxPointer->m_size.y - seq.m_currentSize.y + m_chatboxEdge->m_size.y * 2));

        screenPos.x = utils::clamp(screenPos.x, static_cast<float>(m_edgeGap + m_chatboxEdge->m_size.x + m_chatboxPointer->m_size.x / 2 + 1), static_cast<float>(gamedata::global::hudLayerResolution.x - m_edgeGap - m_chatboxEdge->m_size.x - m_chatboxPointer->m_size.x / 2 - 1));
    }

    m_renderer.fillRectangle(screenPos - Vector2{1, 1}, {2, 2}, {0, 255, 0, 150});

    const Vector2<int> iScreenPos = screenPos;

    float progress = seq.m_windowTimer.getProgressNormalized();

    seq.m_currentSize = utils::lerp(seq.m_oldSize, seq.m_targetSize, progress);

    std::cout << seq.m_currentMessage->m_charDelayTimer.getCurrentFrame() << std::endl;
    for (auto &el : seq.m_currentMessage->m_symbolAppearTimers)
        std::cout << el.getCurrentFrame() << " ";
    std::cout << std::endl;

    if (seq.m_currentSize.x >= m_chatboxPointer->m_size.x)
    {
        if (boxTop)
            m_renderer.renderTexture(m_chatboxPointer->m_id, {iScreenPos.x - m_chatboxPointer->m_size.x / 2, iScreenPos.y - m_chatboxPointer->m_size.y}, m_chatboxPointer->m_size, SDL_FLIP_VERTICAL, 1.0f);
        else
            m_renderer.renderTexture(m_chatboxPointer->m_id, {iScreenPos.x - m_chatboxPointer->m_size.x / 2, iScreenPos.y}, m_chatboxPointer->m_size, SDL_FLIP_NONE, 1.0f);
    }

    Vector2<int> outerBoundTL(iScreenPos.x - seq.m_currentSize.x / 2 - m_chatboxEdge->m_size.x, (boxTop ? iScreenPos.y - m_chatboxPointer->m_size.y - seq.m_currentSize.y - m_chatboxEdge->m_size.y * 2 : iScreenPos.y + m_chatboxPointer->m_size.y) + int(boxTop));
    Vector2<int> outerBoundBR(iScreenPos.x + seq.m_currentSize.x / 2 + m_chatboxEdge->m_size.x, (boxTop ? iScreenPos.y - m_chatboxPointer->m_size.y : iScreenPos.y + m_chatboxPointer->m_size.y + m_chatboxEdge->m_size.y * 2 + seq.m_currentSize.y) + int(boxTop));


    if (outerBoundTL.x < m_edgeGap)
    {
        const int offset = m_edgeGap - outerBoundTL.x;
        outerBoundTL.x += offset;
        outerBoundBR.x += offset;
    }
    else if (outerBoundBR.x > gamedata::global::hudLayerResolution.x - m_edgeGap)
    {
        const int offset = outerBoundBR.x - (gamedata::global::hudLayerResolution.x - m_edgeGap);
        outerBoundTL.x -= offset;
        outerBoundBR.x -= offset;
    }

    //ren.drawRectangle(outerBoundTL, outerBoundBR - outerBoundTL, {255, 0, 0, 255});

    m_renderer.fillRectangle(Vector2(outerBoundTL.x + m_chatboxEdge->m_size.x, outerBoundTL.y), seq.m_currentSize + Vector2{0, m_chatboxEdge->m_size.y * 2}, gamedata::colors::LVL4);
    m_renderer.fillRectangle(Vector2(outerBoundTL.x, outerBoundTL.y + m_chatboxEdge->m_size.y), seq.m_currentSize + Vector2{m_chatboxEdge->m_size.x * 2, 0}, gamedata::colors::LVL4);

    m_renderer.renderTexture(m_chatboxEdge->m_id,
            outerBoundTL,
            m_chatboxEdge->m_size, SDL_FLIP_NONE, 1.0f);

    m_renderer.renderTexture(m_chatboxEdge->m_id,
            {outerBoundBR.x - m_chatboxEdge->m_size.x, outerBoundTL.y},
            m_chatboxEdge->m_size, SDL_FLIP_HORIZONTAL, 1.0f);

    m_renderer.renderTexture(m_chatboxEdge->m_id,
            {outerBoundTL.x, outerBoundBR.y - m_chatboxEdge->m_size.y},
            m_chatboxEdge->m_size, SDL_FLIP_VERTICAL, 1.0f);

    m_renderer.renderTexture(m_chatboxEdge->m_id,
            outerBoundBR - m_chatboxEdge->m_size,
            m_chatboxEdge->m_size, SDL_FlipMode(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL), 1.0f);

    std::cout << (int)seq.m_currentState << std::endl;
    if (seq.m_currentMessage && seq.m_currentState == ChatMessageSequence::BoxState::IDLE)
        renderText(seq, outerBoundTL + Vector2{m_chatboxEdge->m_size.x + ChatConsts::ChatEdgeGap, m_chatboxEdge->m_size.y + ChatConsts::ChatEdgeGap});
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

void ChatMessageSequence::compileAndSetSize()
{
    for (auto &msg : m_messages)
        msg.compileAndSetSize();

    m_targetSize = m_messages[0].m_size;
    m_targetSize.x += ChatConsts::ChatEdgeGap * 2;
    m_targetSize.y += ChatConsts::ChatEdgeGap * 2;

    if (!m_messages.empty())
        m_currentMessage = &m_messages[0];

    m_windowTimer.begin(ChatConsts::fadeInDuration);
}

void ChatMessageSequence::takeInput()
{
    if (m_proceedByInput)
    {
        if (m_currentState == BoxState::IDLE && !m_messages.empty())
        {
            if (m_messages[0].m_currentState == ChatMessage::MessageState::APPEAR)
            {
                m_messages[0].skip();
                m_windowTimer.beginAt(1, 1);
            }
            else if (m_messages[0].m_currentState == ChatMessage::MessageState::IDLE)
            {
                m_messages.erase(m_messages.begin());
                if (m_messages.empty())
                {
                    m_currentState = BoxState::DISAPPEAR;
                    m_targetSize = 0;
                    m_oldSize = m_currentSize;
                    m_windowTimer.begin(ChatConsts::fadeOutDuration);
                }
                else
                {
                    m_currentMessage = &m_messages[0];
                    m_currentMessage->m_charDelayTimer.begin(m_currentMessage->m_characterDelay + 1);
                    m_oldSize = m_currentSize;

                    m_targetSize = m_currentMessage->m_size;
                    m_targetSize.x += ChatConsts::ChatEdgeGap * 2;
                    m_targetSize.y += ChatConsts::ChatEdgeGap * 2;
                    m_windowTimer.begin(ChatConsts::fadeBetweenDuration);
                }
            }
        }
    }
}

const fonts::Symbol *ChatMessageSequence::currentSymbol() const
{
    return m_currentMessage->m_symbols[m_currentMessage->m_currentProceedingCharacter];
}

ChatMessage::ChatMessage(const std::string &textRaw_, int font_) :
    m_textRaw(textRaw_),
    m_baseFont(font_)
{
}

void ChatMessage::compileAndSetSize()
{
    auto &textMan = Application::instance().m_textManager;

    Vector2<int> currentLineSize;

    U8Wrapper wrp(m_textRaw);

    bool newLine = true;

    bool readingChunk = false;
    std::string cmdChunk;

    for (auto &ch : wrp)
    {
        if (readingChunk)
        {
            if (ch.m_byteSize > 1)
                throw std::runtime_error("Impossible char size in dialogue command");

            if (*ch.m_ch == '>')
            {
                readingChunk = false;
                m_techSymbols.emplace_back(processCommand(cmdChunk));
                m_symbols.push_back(m_techSymbols.back().get());
            }
            else
            {
                if (*ch.m_ch != ' ' && *ch.m_ch != '\t')
                    cmdChunk += *ch.m_ch;
            }
        }
        else
        {
            if (ch.m_byteSize == 1 && *ch.m_ch == '<')
            {
                readingChunk = true;
                cmdChunk = "";
                continue;
            }

            auto sym = textMan.getSymbol(m_baseFont, ch.getu8());

            if (newLine)
            {
                currentLineSize.x = -sym->m_minx;
                currentLineSize.y = textMan.getFontHeight(m_baseFont);
                newLine = false;
            }

            if (*ch.m_ch == '\n')
            {
                m_size.y += currentLineSize.y;
                m_size.x = std::max(m_size.x, currentLineSize.x);
                m_lineHeights.push_back(currentLineSize.y);

                currentLineSize.x = 0;
                currentLineSize.y = 0;

                newLine = true;

                m_symbols.push_back(nullptr);
            }
            else
            {
                m_symbols.push_back(sym);
                currentLineSize.x += sym->m_advance;
            }

        }
    }

    m_size.y += currentLineSize.y;
    m_size.x = std::max(m_size.x, currentLineSize.x);
    m_lineHeights.push_back(currentLineSize.y);

    m_symbolAppearTimers.resize(m_symbols.size());
    m_symbolAppearTimers[0].begin(m_appearDuration);
}

void ChatMessage::skip()
{
    m_currentState = MessageState::IDLE;

    for (auto &el : m_symbolAppearTimers)
        el.forceOver();
    m_currentProceedingCharacter = m_symbolAppearTimers.size();
    m_firstCharacterForFadingIn = m_symbolAppearTimers.size();
}

void ChatMessage::proceedUntilNonTechCharacter()
{
    m_currentProceedingCharacter++;
    while (m_currentProceedingCharacter < m_symbols.size())
    {
        if (const auto* techsym = dynamic_cast<const TechSymbol*>(m_symbols[m_currentProceedingCharacter]))
        {
            m_symbolAppearTimers[m_currentProceedingCharacter].beginAt(1, 1);
            auto *p = const_cast<TechSymbol*>(techsym);
            auto res = p->onReached(*this);
            if (!res)
                break;
            m_currentProceedingCharacter++;
        }
        else
            break;
    }
}

SymbolDelay::SymbolDelay(int delay_) :
    m_delay(delay_)
{
}

SymbolSetCharacterSpeed::SymbolSetCharacterSpeed(uint32_t characterDelay_, uint32_t appearDuration_) :
    m_characterDelay(characterDelay_),
    m_appearDuration(appearDuration_)
{
}

bool SymbolDelay::onReached(ChatMessage &message_)
{
    message_.m_charDelayTimer.begin(m_delay);
    return false;
}

bool IRenderSymbol::onReached(ChatMessage&)
{
    return true;
}

void TechSymbol::onRenderReached(ChatMessageSequence&)
{
}

bool SymbolSetCharacterSpeed::onReached(ChatMessage &message_)
{
    message_.m_characterDelay = m_characterDelay;
    message_.m_appearDuration = m_appearDuration;
    return false;
}

SymbolRenderShake::SymbolRenderShake(int xAmp_, int yAmp_, float prob_) :
    m_xAmp(xAmp_),
    m_yAmp(yAmp_),
    m_prob(prob_)
{
}

Vector2<int> SymbolRenderShake::getOffset() const
{
    float roll = (rand() % 10000) / 10000.0f;
    if (roll >= m_prob)
        return {};

    return Vector2<int>(
        (rand() % m_xAmp) - m_xAmp / 2,
        (rand() % m_yAmp) - m_yAmp / 2
    );
}

InlinedValueHandler::InlinedValueHandler(const std::string &s_) :
    m_tokens(utils::tokenize(s_, ','))
{
}
