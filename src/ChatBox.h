#ifndef CHATBOX_H_
#define CHATBOX_H_
#include "Application.h"
#include "CoreComponents.h"
#include <entt/entt.hpp>

enum class ChatBoxSide
{
    STRICT_TOP, // Always on top even if it has to leave the screen
    STRICT_BOTTOM, // Always in the bottom even if it has to leave the screen
    PREFER_TOP, // On top as long as it fits into the screen
    PREFER_BOTTOM, // On top as long as it fits into the screen
    AUTO // Wherever there is more free space
};

struct ChatMessage
{
    std::string m_textRaw;
    int m_baseFont;
    std::vector<const fonts::Symbol*> m_symbols;
    std::vector<FrameTimer<false>> m_symbolAppearTimers;
    std::vector<int> m_lineHeights;
    uint32_t m_defaultCharacterDelay = 2;
    uint32_t m_defaultAppearDuration = 12;
    uint32_t m_currentProceedingCharacter = 0;
    uint32_t m_firstCharacterForFadingIn = 0;
    enum class State { APPEAR, IDLE } m_currentState = State::APPEAR;

    Vector2<int> compileAndGetSize(const TextManager &textMan_);
};

struct ChatMessageSequence
{
    ChatMessageSequence(entt::entity src_, const ChatBoxSide &side_, bool fitScreen_, bool proceedByInput_, bool claimInputs_, bool returnInputs_);
    void compileAndSetSize(const TextManager &textMan_);

    entt::entity m_source;
    Vector2<int> m_oldSize;
    Vector2<int> m_currentSize;
    Vector2<int> m_targetSize;
    FrameTimer<true> m_timer;
    FrameTimer<true> m_characterTimer;
    ChatBoxSide m_side;
    bool m_fitScreen;

    bool m_proceedByInput = false;
    bool m_claimInputs = false;
    bool m_returnInputs = false;

    std::vector<ChatMessage> m_messages;
    ChatMessage *m_currentMessage = nullptr;

};

class ChatboxSystem : public InputReactor
{
public:
    ChatboxSystem(entt::registry &reg_, Application &app_, Camera &camera_);
    
    void setPlayerEntity(entt::entity playerId_);

    void addSequence(ChatMessageSequence &&seq_);
    void receiveEvents(HUD_EVENTS event, const float scale_) override;

    void endSequence(size_t seqId_);

    void renderText(ChatMessageSequence &seq_, const Vector2<int> &tl_);

    void draw();

private:
    entt::registry &m_reg;
    Application &m_app;
    Camera &m_camera;
    std::vector<ChatMessageSequence> m_sequences;

    entt::entity m_playerId;

    Texture_t m_chatboxEdge;
    Texture_t m_chatboxPointer;

    const int m_edgeGap = 6;
};

#endif
