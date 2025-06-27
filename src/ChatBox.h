#ifndef CHATBOX_H_
#define CHATBOX_H_
#include "Application.h"
#include "Texture.h"
#include "CoreComponents.h"
#include <entt/entt.hpp>
#include <stack>

enum class ChatBoxSide
{
    STRICT_TOP, // Always on top even if it has to leave the screen
    STRICT_BOTTOM, // Always in the bottom even if it has to leave the screen
    PREFER_TOP, // On top as long as it fits into the screen
    PREFER_BOTTOM, // On top as long as it fits into the screen
    AUTO // Wherever there is more free space
};

class ChatMessage;
class ChatMessageSequence;

class InlinedValueHandler
{
public:
    InlinedValueHandler(const std::string &s_);

    template<typename T>
    T getParam(int index_);

    template<typename T>
    T getParam(int index_, const T &default_);

private:
    std::vector<std::string> m_tokens;
};

class TechSymbol : public fonts::Symbol
{
public:
    // Symbol was naturally reached by message, returns true if should proceed further
    virtual bool onReached(ChatMessage &message_) = 0;

    // Symbol was naturally reached by sequence while rendering
    virtual void onRenderReached(ChatMessageSequence &sequence_);
};

class IRenderSymbol : public TechSymbol
{
public:
    virtual bool onReached(ChatMessage &message_) override;

    // Symbol was naturally reached by sequence while rendering
    virtual void onRenderReached(ChatMessageSequence &sequence_) = 0;
};

// Add itself to the sequence (which keeps current rendering state)
template<typename OwnT>
class RenderSymbol : public IRenderSymbol
{
public:
    virtual void onRenderReached(ChatMessageSequence &sequence_) override;
};

// Remove actual symbol from the sequence (which keeps current rendering state)
template<typename OwnT>
class RenderDropSymbol : public IRenderSymbol
{
public:
    virtual void onRenderReached(ChatMessageSequence &sequence_) override;
};

/*
    <delay=30>
*/
class SymbolDelay : public TechSymbol
{
public:
    SymbolDelay(int delay_);

    virtual bool onReached(ChatMessage &message_) override;

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

    virtual bool onReached(ChatMessage &message_) override;

private:
    uint32_t m_characterDelay = 0;
    uint32_t m_appearDuration = 0;
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


struct ChatMessage
{
    ChatMessage(const std::string &textRaw_, int font_);
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
    uint32_t m_currentProceedingCharacter = 0;
    uint32_t m_firstCharacterForFadingIn = 0;

    enum class MessageState { APPEAR, IDLE } m_currentState = MessageState::APPEAR;

    void compileAndSetSize(const TextManager &textMan_);
    void skip();

    void proceedUntilNonTechCharacter();
};

struct ChatMessageSequence
{
    ChatMessageSequence(entt::entity src_, const ChatBoxSide &side_, bool fitScreen_, bool proceedByInput_, bool claimInputs_, bool returnInputs_);
    ChatMessageSequence(ChatMessageSequence &&) = default;
    ChatMessageSequence &operator=(ChatMessageSequence &&) = default;
    void compileAndSetSize(const TextManager &textMan_);
    void takeInput();
    const fonts::Symbol* currentSymbol() const;

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

    enum class BoxState { APPEAR, IDLE, DISAPPEAR } m_currentState = BoxState::APPEAR;

    std::vector<ChatMessage> m_messages;
    ChatMessage *m_currentMessage = nullptr;

    std::tuple<std::stack<SymbolRenderShake*>> m_renderEffects;
};

class ChatboxSystem : public InputReactor
{
public:
    ChatboxSystem(entt::registry &reg_, Application &app_, Camera &camera_);
    
    void setPlayerEntity(entt::entity playerId_);

    void addSequence(ChatMessageSequence &&seq_);
    void receiveEvents(HUD_EVENTS event, const float scale_) override;

    void renderText(ChatMessageSequence &seq_, const Vector2<int> &tl_);

    void update();
    void draw();

private:
    entt::registry &m_reg;
    Application &m_app;
    Camera &m_camera;
    std::vector<ChatMessageSequence> m_sequences;

    entt::entity m_playerId;

    std::shared_ptr<Texture> m_chatboxEdge;
    std::shared_ptr<Texture> m_chatboxPointer;

    const int m_edgeGap = 6;
};

template<typename OwnT>
void RenderSymbol<OwnT>::onRenderReached(ChatMessageSequence &sequence_)
{
    std::get<std::stack<OwnT*>>(sequence_.m_renderEffects).push(dynamic_cast<OwnT*>(this));
}

template<typename OwnT>
void RenderDropSymbol<OwnT>::onRenderReached(ChatMessageSequence &sequence_)
{
    std::get<std::stack<OwnT*>>(sequence_.m_renderEffects).pop();
}

#endif
