#ifndef CHATBOX_H_
#define CHATBOX_H_
#include "Core/FrameTimer.h"
#include "Core/InputSystem.h"
#include "Core/Texture.h"
#include "Core/TextManager.h"
#include <entt/entt.hpp>
#include <stack>

enum class ChatBoxSide : uint8_t
{
    STRICT_TOP, // Always on top even if it has to leave the screen
    STRICT_BOTTOM, // Always in the bottom even if it has to leave the screen
    PREFER_TOP, // On top as long as it fits into the screen
    PREFER_BOTTOM, // On top as long as it fits into the screen
    AUTO // Wherever there is more free space
};

struct ChatMessage;
struct ChatMessageSequence;
class SymbolRenderShake;

class InlinedValueHandler
{
public:
    InlinedValueHandler(const std::string &s_);

    template<typename T>
    T getParam(size_t index_);

    template<typename T>
    T getParam(size_t index_, const T &default_);

private:
    std::vector<std::string> m_tokens;
};

struct ChatMessage
{
    ChatMessage(std::string &&textRaw_, int font_);
    ChatMessage(ChatMessage &&rhs_) = default;
    ChatMessage &operator=(ChatMessage &&rhs_) = default;
    std::string m_textRaw;
    int m_baseFont;
    std::vector<const fonts::Symbol*> m_symbols;
    std::vector<FrameTimer<false>> m_symbolAppearTimers;
    std::vector<int> m_lineHeights;
    std::vector<std::unique_ptr<fonts::Symbol>> m_techSymbols;

    Vector2<int> m_size;

    // Timer for delay between characters
    FrameTimer<true> m_charDelayTimer;

    static uint32_t m_defaultCharacterDelay;
    static uint32_t m_defaultAppearDuration;

    uint32_t m_characterDelay = m_defaultCharacterDelay;
    uint32_t m_appearDuration = m_defaultAppearDuration;
    size_t m_currentProceedingCharacter = 0;
    size_t m_firstCharacterForFadingIn = 0;

    enum class MessageState : uint8_t
    {
        APPEAR,
        IDLE
    } m_currentState = MessageState::APPEAR;

    void compileAndSetSize();
    void skip();

    void proceedUntilNonTechCharacter();
};

struct ChatMessageSequence
{
public:
    ChatMessageSequence(entt::entity src_, const ChatBoxSide &side_, bool fitScreen_, bool proceedByInput_, bool claimInputs_, bool returnInputs_);
    ChatMessageSequence(ChatMessageSequence &&) noexcept = default;
    ChatMessageSequence &operator=(ChatMessageSequence &&) noexcept = default;
    void compileAndSetSize();
    void takeInput();
    const fonts::Symbol* currentSymbol() const;

    void update();

    bool isMarkedForDeletion() const noexcept;

    bool empty() const;
    const ChatMessage &currentMessage() const;
    void addMessage(ChatMessage &&message_);

    entt::entity m_source;
    Vector2<int> m_oldSize;
    Vector2<int> m_currentSize;
    Vector2<int> m_targetSize;
    ChatBoxSide m_side;
    bool m_fitScreen;

    // Timer for box appear / disappear animation
    FrameTimer<true> m_windowTimer;

    // Progress with inputs
    bool m_proceedByInput = false;

    // Disable inputs for the player when the sequence begins
    bool m_claimInputs = false;

    // Return inputs to the player once the sequence is over
    bool m_returnInputs = false;

    enum class BoxState : uint8_t { APPEAR, IDLE, DISAPPEAR } m_currentState = BoxState::APPEAR;

    /*
        Effects applied while rendering
        Multiple effects can be applied at once, but only the last version of each effect is applied
    */
    using RenderEffects = std::tuple<std::stack<SymbolRenderShake*>>;

private:
    std::vector<ChatMessage> m_messages;
    ChatMessage *m_currentMessage = nullptr;
    bool m_toBeDeleted = false;
};

class ChatboxSystem : public InputReactor
{
public:
    ChatboxSystem(entt::registry &reg_, Camera &camera_);
    
    void setPlayerEntity(entt::entity playerId_);

    void addSequence(ChatMessageSequence &&seq_);
    void receiveEvents(HUD_EVENTS event, float scale_) override;

    void renderMessage(const ChatMessage &msg_, const Vector2<int> &tl_) const;

    void update();
    void draw() const;

private:
    entt::registry &m_reg;
    Camera &m_camera;
    Renderer &m_renderer;
    std::vector<ChatMessageSequence> m_sequences;

    entt::entity m_playerId = entt::null;

    std::shared_ptr<Texture> m_chatboxEdge;
    std::shared_ptr<Texture> m_chatboxPointer;

    const int m_edgeGap = 6;
};


/*
    Abstract technical symbol
    Applies any sort of effect that might be handled during the iteration or during the rendering
*/
class TechSymbol : public fonts::Symbol
{
public:
    // Symbol was naturally reached by message, returns true if should proceed further
    virtual bool onReached(ChatMessage&) = 0;

    // Symbol was naturally reached by sequence while rendering
    virtual void onRenderReached(ChatMessageSequence::RenderEffects&);
};

/*
    Abstract rendering symbol
    Since it applies an effect to be handled while rendering, it doesn't need an onReached() implementation
*/
class IRenderSymbol : public TechSymbol
{
public:
    bool onReached(ChatMessage&) override;
};


/*
    Base class for all rendering symbols
    Add itself to the sequence using CRTP (which keeps current rendering state)
*/
template<typename OwnT>
class RenderSymbol : public IRenderSymbol
{
public:
    void onRenderReached(ChatMessageSequence::RenderEffects &effects) override;

private:
    RenderSymbol() = default;

    friend SymbolRenderShake;
};

/*
    Base class for all closing rendering symbols
    Removes last symbol of that type from the sequence using CRTP (which keeps current rendering state)
*/
template<typename OwnT>
class RenderDropSymbol : public IRenderSymbol
{
public:
    void onRenderReached(ChatMessageSequence::RenderEffects &effects) override;
};


/*
    shake = xAmp, yAmp
    <shake=4,6>
*/
class SymbolRenderShake : public RenderSymbol<SymbolRenderShake>
{
public:
    SymbolRenderShake(int xAmp_, int yAmp_, float prob_);

    Vector2<int> getOffset() const;

private:
    int m_xAmp;
    int m_yAmp;
    float m_prob;
};

/*
    <delay=30>
*/
class SymbolDelay : public TechSymbol
{
public:
    SymbolDelay(int delay_);

    bool onReached(ChatMessage &message_) override;

private:
    int m_delay = 0;
};

/*
    shake = chardelay, appearDuration
    <charspd=5,15>
    <charspd=5,default>
    <charspd=default,default>
*/
class SymbolSetCharacterSpeed : public TechSymbol
{
public:
    SymbolSetCharacterSpeed(uint32_t characterDelay_, uint32_t appearDuration_);

    bool onReached(ChatMessage &message_) override;

private:
    uint32_t m_characterDelay = 0;
    uint32_t m_appearDuration = 0;
};


template<typename OwnT>
void RenderSymbol<OwnT>::onRenderReached(ChatMessageSequence::RenderEffects &effects)
{
    std::get<std::stack<OwnT*>>(effects).push(dynamic_cast<OwnT*>(this));
}

template<typename OwnT>
void RenderDropSymbol<OwnT>::onRenderReached(ChatMessageSequence::RenderEffects &effects)
{
    std::get<std::stack<OwnT*>>(effects).pop();
}

#endif
