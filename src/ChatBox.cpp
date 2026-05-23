#include "Core/Application.h"
#include "Core/CoreComponents.h"
#include "Core/GameData.h"
#include "Core/InputResolver.h"
#include "Core/Vector2.hpp"
#include "Core/utf8.h"
#include <memory>
#include <stdexcept>
#include "ChatBox.h"

const uint32_t CharAppearanceSpeed::defaultDelay = 2;
const uint32_t CharAppearanceSpeed::defaultDuration = 12;

const uint8_t ChatSymbol::Effects::Shake::defaultXAmp = 0;
const uint8_t ChatSymbol::Effects::Shake::defaultYAmp = 0;
const float ChatSymbol::Effects::Shake::defaultProb = 0.0f;

namespace ChatConsts
{
    const uint32_t fadeInDuration = 5;
    const uint32_t fadeBetweenDuration = 5;
    const uint32_t fadeOutDuration = 7;
}

template <>
std::string InlinedValueHandler::getParam<std::string>(size_t index_)
{
    return m_tokens[index_];
}

template <>
std::string InlinedValueHandler::getParam<std::string>(size_t index_, const std::string &default_)
{
    if (index_ >= m_tokens.size() || m_tokens[index_] == "default")
        return default_;

    return m_tokens[index_];
}

template <>
int InlinedValueHandler::getParam<int>(size_t index_)
{
    return std::stoi(m_tokens[index_]);
}

template <>
int InlinedValueHandler::getParam<int>(size_t index_, const int &default_)
{
    if (index_ >= m_tokens.size() || m_tokens[index_] == "default")
        return default_;

    return std::stoi(m_tokens[index_]);
}

template <>
uint8_t InlinedValueHandler::getParam<uint8_t>(size_t index_)
{
    return std::stoi(m_tokens[index_]);
}

template <>
uint8_t InlinedValueHandler::getParam<uint8_t>(size_t index_, const uint8_t &default_)
{
    if (index_ >= m_tokens.size() || m_tokens[index_] == "default")
        return default_;

    return std::stoi(m_tokens[index_]);
}

template <>
uint32_t InlinedValueHandler::getParam<uint32_t>(size_t index_)
{
    return std::stoi(m_tokens[index_]);
}

template <>
uint32_t InlinedValueHandler::getParam<uint32_t>(size_t index_, const uint32_t &default_)
{
    if (index_ >= m_tokens.size() || m_tokens[index_] == "default")
        return default_;

    return std::stoi(m_tokens[index_]);
}

template <>
float InlinedValueHandler::getParam<float>(size_t index_, const float &default_)
{
    if (index_ >= m_tokens.size() || m_tokens[index_] == "default")
        return default_;

    return std::stof(m_tokens[index_]);
}


ChatSymbol::RenderData::RenderData(const fonts::Symbol &symbol_) :
    id{symbol_.m_id},
    size{symbol_.m_size},
    advance{symbol_.m_advance}
{}

ChatSymbol::ChatSymbol(const fonts::Symbol &symbol_, CharAppearanceSpeed speed_, uint32_t extraDelay_, const Effects &effects_) :
    renderData{symbol_},
    appearanceDelay{speed_.delay + extraDelay_},
    appearanceDuration{speed_.duration},
    effects{effects_}
{}

Line::Line(int minx_, int height_) :
    minx{minx_},
    m_height{height_},
    m_width{minx_}
{}

void Line::addSymbol(ChatSymbol &&sym_)
{
    m_symbols.emplace_back(std::move(sym_));
    m_width += sym_.renderData.advance;
}

int Line::height() const noexcept
{
    return m_height;
}

int Line::width() const noexcept
{
    return m_width;
}

ChatSymbol &Line::operator[](size_t id_)
{
    assert(id_ < m_symbols.size());
    return m_symbols[id_];
}

size_t Line::size() const noexcept
{
    return m_symbols.size();
}

std::vector<ChatSymbol> &Line::symbols() noexcept
{
    return m_symbols;
}

const std::vector<ChatSymbol> &Line::symbols() const noexcept
{
    return m_symbols;
}

ChatMessage::ChatMessage(const std::string &str_)
{
    auto &textMan = Application::instance().m_textManager;

    auto &font = textMan.getFont(Fonts::CHATBOX);

    uint32_t extraDelay = 0;

    // Effect stacks
    std::stack<ChatSymbol::Effects::Shake> shakeStack;
    shakeStack.emplace();

    std::stack<CharAppearanceSpeed> speedStack;
    speedStack.emplace();

    // Parsing
    U8Wrapper wrp(str_);

    bool newLine = true;

    bool readingCmd = false;
    std::string cmd;

    for (auto &ch : wrp)
    {
        if (readingCmd)
        {
            if (ch.m_byteSize > 1)
                throw std::runtime_error("Impossible char size in dialogue command");

            if (*ch.m_ch == '>')
            {
                readingCmd = false;
                InlinedValueHandler parser(cmd);
                if (parser.getCommand() == "charspd")
                    speedStack.emplace(parser.getParam<uint32_t>(0, CharAppearanceSpeed::defaultDelay), parser.getParam<uint32_t>(1, CharAppearanceSpeed::defaultDuration));
                else if (parser.getCommand() == "/charspd")
                    speedStack.pop();
                else if (parser.getCommand() == "shake")
                    shakeStack.emplace(
                        parser.getParam<uint8_t>(0, ChatSymbol::Effects::Shake::defaultXAmp), 
                        parser.getParam<uint8_t>(1, ChatSymbol::Effects::Shake::defaultYAmp),
                        parser.getParam<float>(2, ChatSymbol::Effects::Shake::defaultProb));
                else if (parser.getCommand() == "/shake")
                    shakeStack.pop();
                else if (parser.getCommand() == "delay")
                    extraDelay = parser.getParam<uint8_t>(0);
                else
                    throw std::runtime_error(std::format("Unknown command \"{}\"", cmd));
            }
            else if (*ch.m_ch != ' ' && *ch.m_ch != '\t')
                cmd += *ch.m_ch;
        }
        else
        {
            if (ch.m_byteSize == 1 && *ch.m_ch == '<')
            {
                readingCmd = true;
                cmd = "";
                continue;
            }

            const auto &sym = font[ch.getu8()];

            if (newLine)
            {
                m_lines.emplace_back(sym.m_minx, font.height());
                newLine = false;
            }

            if (*ch.m_ch == '\n')
                newLine = true;
            else
            {
                m_lines.back().addSymbol(ChatSymbol{sym, speedStack.top(), extraDelay, ChatSymbol::Effects{shakeStack.top()}});
                extraDelay = 0;
            }
        }
    }

    for (const auto &line : m_lines)
    {
        m_size.y += line.height();
        m_size.x = std::max(m_size.x, line.width());
    }
}

bool ChatMessage::update()
{
    if (m_lineForDelay < m_lines.size() && m_symbolForDelay < m_lines[m_lineForDelay].size())
    {
        if (m_lines[m_lineForDelay][m_symbolForDelay].appearanceDelay.update())
        {
            if (m_symbolForDelay + 1 < m_lines[m_lineForDelay].size())
                ++m_symbolForDelay;
            else
            {
                m_symbolForDelay = 0;
                ++m_lineForDelay;
            }
        }
    }

    bool ret = m_lineForDelay >= m_lines.size();

    for (size_t line = 0; line < m_lineForDelay; ++line)
    {
        for (auto & symbol : m_lines[line].symbols())
            ret = symbol.appearanceDuration.update() && ret;
    }

    if (m_lineForDelay < m_lines.size())
    {
        for (size_t symbol = 0; symbol < m_symbolForDelay; ++symbol)
            ret = m_lines[m_lineForDelay][symbol].appearanceDuration.update() && ret;
    }

    return ret;
}

void ChatMessage::skip()
{
    for (auto &line : m_lines)
    {
        for (auto &sym : line.symbols())
        {
            sym.appearanceDelay.finish();
            sym.appearanceDuration.finish();
        }
    }
    m_symbolForDelay = 0;
    m_lineForDelay = m_lines.size();
}

Vector2<int> ChatMessage::size() const noexcept
{
    return m_size;
}

const std::vector<Line> &ChatMessage::lines() const noexcept
{
    return m_lines;
}


ChatMessageSequence::ChatMessageSequence(entt::entity source_, ChatBoxSide side_, bool fitScreen_) :
    m_side{side_},
    m_source{source_},
    m_timer{ChatConsts::fadeInDuration},
    m_fitScreen{fitScreen_}
{}

void ChatMessageSequence::addMessage(const std::string &message_)
{
    m_messages.emplace_back(message_);
    if (m_messages.size() == 1)
        newSize = m_messages.front().size();
}

bool ChatMessageSequence::empty() const noexcept
{
    return m_messages.empty();
}

bool ChatMessageSequence::hasMessagesLeft() const noexcept
{
    return m_currentMessage < m_messages.size();
}

const ChatMessage &ChatMessageSequence::message() const noexcept
{
    assert(m_currentMessage <  m_messages.size());
    return m_messages[m_currentMessage];
}

bool ChatMessageSequence::update()
{
    switch (m_currentState)
    {
        case State::APPEAR:
            if (m_timer.update())
                m_currentState = State::PRINTING;
        break;

        case State::PRINTING:
            m_timer.update();
            if (m_currentMessage < m_messages.size() && m_messages[m_currentMessage].update())
                m_currentState = State::IDLE;
        break;

        case State::IDLE:
            m_timer.update();
            m_messages[m_currentMessage].update();
        break;

        case State::DISAPPEAR:
            if (m_timer.update())
                return true;
        break;
    }

    return false;
}

void ChatMessageSequence::proceed()
{
    switch (m_currentState)
    {
        case State::APPEAR:
            [[fallthrough]];
        case State::PRINTING:
            if (m_currentMessage < m_messages.size())
                m_messages[m_currentMessage].skip();
            m_timer.finish();
            m_currentState = State::IDLE;
        break;

        case State::IDLE:
            ++m_currentMessage;
            if (m_currentMessage >= m_messages.size())
            {
                lastSize = currentSize();
                newSize = {0, 0};
                m_currentState = State::DISAPPEAR;
                m_timer.begin(ChatConsts::fadeOutDuration);
            }
            else
            {
                lastSize = currentSize();
                newSize = m_messages[m_currentMessage].size();
                m_currentState = State::PRINTING;
                m_timer.begin(ChatConsts::fadeBetweenDuration);
            }
        break;

        case State::DISAPPEAR:
        break;
    }
}

ChatBoxSide ChatMessageSequence::side() const noexcept
{
    return m_side;
}

entt::entity ChatMessageSequence::source() const
{
    return m_source;
}

Vector2<int> ChatMessageSequence::currentSize() const noexcept
{
    return utils::lerp(lastSize, newSize, m_timer.getProgressNormalized());
}

bool ChatMessageSequence::fitScreen() const noexcept
{
    return m_fitScreen;
}

SequenceRenderer::SequenceRenderer(const ChatMessageSequence &seq_, const entt::registry &reg_, const Camera &cam_, const Texture &chatboxEdge_, const Texture &chatboxPointer_) :
    m_renderer{Application::instance().m_renderer},
    m_seq{seq_},
    m_reg{reg_},
    m_cam{cam_},
    m_chatboxEdge{chatboxEdge_},
    m_chatboxPointer{chatboxPointer_}
{}

void SequenceRenderer::draw()
{
    calcPos();
    drawImpl();
}

void SequenceRenderer::calcPos()
{
    m_internalSize = m_seq.currentSize().add(m_edgeGap * 2, m_edgeGap * 2);
    m_externalSize = m_internalSize + m_chatboxEdge.size() * 2;

    const auto &srcpoint = m_reg.get<HUDPoint>(m_seq.source());
    auto worldPos = srcpoint.m_pos;
    if (srcpoint.m_posRule == HUDPosRule::REL_TRANSFORM)
    {
        worldPos += m_reg.get<ComponentTransform>(m_seq.source()).m_pos;
    }

    const auto camSize = m_cam.getSize();

    if (m_seq.side() == ChatBoxSide::STRICT_TOP)
    {
        m_boxTop = true;
    }
    else if (m_seq.side() == ChatBoxSide::STRICT_BOTTOM)
    {
        m_boxTop = false;
    }
    else if (m_seq.side() == ChatBoxSide::PREFER_TOP)
    {
        const auto worldCamPosY = worldPos.y - srcpoint.m_vOffset - m_cam.getTopLeft().y;
        const auto screenPosY = worldCamPosY / camSize.y * gamedata::global::hudLayerResolution.y;

        if (screenPosY >= m_externalSize.y + m_chatboxPointer.size().y)
            m_boxTop = true;
    }
    else if (m_seq.side() == ChatBoxSide::PREFER_BOTTOM)
    {
        const auto worldCamPosY = worldPos.y - srcpoint.m_vOffset - m_cam.getTopLeft().y;
        const auto screenPosY = worldCamPosY / camSize.y * gamedata::global::hudLayerResolution.y;

        if (screenPosY > gamedata::global::hudLayerResolution.y - m_externalSize.y - m_chatboxPointer.size().y)
            m_boxTop = true;
    }
    else if (m_seq.side() == ChatBoxSide::AUTO)
    {
        auto screenPos = worldPos - m_cam.getTopLeft();
        screenPos.y = screenPos.y / camSize.y * gamedata::global::hudLayerResolution.y;

        m_boxTop = screenPos.y > (gamedata::global::hudLayerResolution.y - screenPos.y);
    }

    if (m_boxTop)
        worldPos.y -= srcpoint.m_vOffset;
    else
        worldPos.y += srcpoint.m_vOffset;

    m_screenPos = worldPos - m_cam.getTopLeft();
    m_screenPos.x = m_screenPos.x / camSize.x * gamedata::global::hudLayerResolution.x;
    m_screenPos.y = m_screenPos.y / camSize.y * gamedata::global::hudLayerResolution.y;

    if (m_seq.fitScreen())
    {
        if (m_boxTop)
            m_screenPos.y = utils::clamp<int>(m_screenPos.y, m_chatboxPointer.size().y + m_externalSize.y, gamedata::global::hudLayerResolution.y);
        else
            m_screenPos.y = utils::clamp<int>(m_screenPos.y, 0, gamedata::global::hudLayerResolution.y - m_chatboxPointer.size().y - m_externalSize.y);

        m_screenPos.x = utils::clamp(m_screenPos.x, m_externalSize.x / 2, gamedata::global::hudLayerResolution.x - m_externalSize.x / 2);
    }

    m_outerBoundTL.x = m_screenPos.x - m_externalSize.x / 2;
    m_outerBoundTL.y = (m_boxTop ? 
        m_screenPos.y - m_chatboxPointer.size().y - m_externalSize.y : 
        m_screenPos.y + m_chatboxPointer.size().y);

    m_outerBoundBR = m_outerBoundTL + m_externalSize;
}

void SequenceRenderer::drawImpl() const
{
    // Draw pointer
    if (m_seq.currentSize().x >= m_chatboxPointer.size().x)
    {
        if (m_boxTop)
            m_renderer.renderTexture(m_chatboxPointer.handler(), {m_screenPos.x - m_chatboxPointer.size().x / 2, m_screenPos.y - m_chatboxPointer.size().y}, m_chatboxPointer.size(), SDL_FLIP_VERTICAL, 1.0f);
        else
            m_renderer.renderTexture(m_chatboxPointer.handler(), {m_screenPos.x - m_chatboxPointer.size().x / 2, m_screenPos.y}, m_chatboxPointer.size(), SDL_FLIP_NONE, 1.0f);
    }

    // Draw crossing rectangles
    m_renderer.fillRectangle(Vector2(m_outerBoundTL.x + m_chatboxEdge.size().x, m_outerBoundTL.y), Vector2{m_internalSize.x, m_externalSize.y}, gamedata::colors::LVL4);
    m_renderer.fillRectangle(Vector2(m_outerBoundTL.x, m_outerBoundTL.y + m_chatboxEdge.size().y), Vector2{m_externalSize.x, m_internalSize.y}, gamedata::colors::LVL4);

    // Draw corners
    m_renderer.renderTexture(m_chatboxEdge.handler(),
            m_outerBoundTL,
            m_chatboxEdge.size(), SDL_FLIP_NONE, 1.0f);

    m_renderer.renderTexture(m_chatboxEdge.handler(),
            {m_outerBoundBR.x - m_chatboxEdge.size().x, m_outerBoundTL.y},
            m_chatboxEdge.size(), SDL_FLIP_HORIZONTAL, 1.0f);

    m_renderer.renderTexture(m_chatboxEdge.handler(),
            {m_outerBoundTL.x, m_outerBoundBR.y - m_chatboxEdge.size().y},
            m_chatboxEdge.size(), SDL_FLIP_VERTICAL, 1.0f);

    m_renderer.renderTexture(m_chatboxEdge.handler(),
            m_outerBoundBR - m_chatboxEdge.size(),
            m_chatboxEdge.size(), SDL_FlipMode(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL), 1.0f);

    // Render message
    if (m_seq.hasMessagesLeft())
        drawMessageImpl(m_seq.message());

#if 0 // debugging
    m_renderer.drawRectangle(m_outerBoundTL, m_outerBoundBR - m_outerBoundTL, {255, 0, 0, 150});
    m_renderer.drawRectangle(m_outerBoundTL + m_chatboxEdge.size(), m_outerBoundBR - m_outerBoundTL - m_chatboxEdge.size() * 2, {255, 0, 0, 150});
    m_renderer.drawRectangle(m_outerBoundTL + m_chatboxEdge.size().add(m_edgeGap, m_edgeGap), m_outerBoundBR - m_outerBoundTL.add(m_edgeGap * 2, m_edgeGap * 2) - m_chatboxEdge.size() * 2, {255, 0, 0, 150});
#endif
}

void SequenceRenderer::drawMessageImpl(const ChatMessage &msg_) const
{
    const auto topLeft = m_outerBoundTL + m_chatboxEdge.size() + Vector2{m_edgeGap, m_edgeGap};

    auto pos = topLeft;
    const auto &lines = msg_.lines();
    for (const auto & line : lines)
    {
        pos.x -= line.minx;

        for (const auto &sym : line.symbols())
        {
            if (!sym.appearanceDelay.isOver())
                return;

            Vector2<int> offset;

            // handling shake
            float roll = static_cast<float>(rand() % 10000) / 10000.0f;
            if (roll < sym.effects.shake.prob)
            {
                if (sym.effects.shake.xAmp > 0)
                    offset.x = (rand() % sym.effects.shake.xAmp) - sym.effects.shake.xAmp / 2;

                if (sym.effects.shake.yAmp)
                    offset.y = (rand() % sym.effects.shake.yAmp) - sym.effects.shake.yAmp / 2;
            }

            const auto progress = Easing::circ(sym.appearanceDuration.getProgressNormalized());
            m_renderer.renderTexture(sym.renderData.id, Vector2{pos.x, pos.y - 5 + int(5 * progress)} + offset, sym.renderData.size, SDL_FLIP_NONE, progress);
            pos.x += sym.renderData.advance;
        }

        pos.x = topLeft.x;
        pos.y += line.height();
    }
}


ChatboxSystem::ChatboxSystem(entt::registry &reg_, Camera &camera_, InputHandlingSystem &inputSystem_) :
    m_reg(reg_),
    m_camera(camera_),
    m_renderer(Application::instance().m_renderer),
    m_inputSystem(inputSystem_)
{
    subscribe(HUD_EVENTS::PROCEED);
    setInputEnabled();

    auto &texman = Application::instance().m_textureManager;

    m_chatboxEdge = texman.getTexture(texman.getTexID("UI/chatbox_edge"));
    m_chatboxPointer = texman.getTexture(texman.getTexID("UI/chatbox_pointer"));
}

void ChatboxSystem::addSequence(ChatMessageSequence &&seq_)
{
    if (seq_.empty())
        throw std::runtime_error("ChatboxSystem received an empty sequence");

    m_sequences.emplace_back(std::move(seq_));

/*
    if (m_sequences.size() == 1 && m_sequences[0].m_claimInputs)
    {
        m_inputSystem.deactivate();
    }
*/
}

void ChatboxSystem::update()
{
    size_t i = 0;
    while (i < m_sequences.size())
    {
        if (m_sequences[i].update())
            m_sequences.erase(m_sequences.begin() + i);
        else
            ++i;
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
                    seq.proceed();
            }
            break;

        default:
        break;
    }
}

void ChatboxSystem::draw() const
{
    for (const auto &seq : m_sequences)
    {
        SequenceRenderer{seq, m_reg, m_camera, *m_chatboxEdge, *m_chatboxPointer}.draw();
    }
}

InlinedValueHandler::InlinedValueHandler(const std::string &s_)
{
    const auto pos = s_.find_first_of('=');
    if (pos == std::string::npos)
        m_command = s_;
    else
    {
        m_command = s_.substr(0, pos);
        m_tokens = utils::tokenize(s_.substr(pos + 1), ',');
    }
}

const std::string &InlinedValueHandler::getCommand() const noexcept
{
    return m_command;
}
