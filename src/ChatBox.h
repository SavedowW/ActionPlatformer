#pragma once
#include "Core/FrameTimer.h"
#include "InputHandlingSystem.h"
#include "Core/Texture.h"
#include "Core/TextManager.h"
#include <entt/entt.hpp>
#include <stack>

// TODO: store last used direction in sequence, don't change it if not necessary
enum class ChatBoxSide : uint8_t
{
    STRICT_TOP, // Always on top even if it has to leave the screen
    STRICT_BOTTOM, // Always in the bottom even if it has to leave the screen
    PREFER_TOP, // On top as long as it fits into the screen
    PREFER_BOTTOM, // On top as long as it fits into the screen
    AUTO // Wherever there is more free space
};

/**
    Utility to parse sequences of comma-separated tokens
    Takes line like "8,default,3", lets you extract individual elements by indices.
    "default" is a key word that forces it to pick default value.
*/
class InlinedValueHandler
{
public:
    InlinedValueHandler(const std::string &s_);

    template<typename T>
    T getParam(size_t index_);

    template<typename T>
    T getParam(size_t index_, const T &default_);

    const std::string &getCommand() const noexcept;

private:
    std::string m_command;
    std::vector<std::string> m_tokens;
};

struct CharAppearanceSpeed
{
    static const uint32_t defaultDelay;
    static const uint32_t defaultDuration;

    uint32_t delay = defaultDelay;
    uint32_t duration = defaultDuration;
};

struct ChatSymbol
{
    const struct RenderData
    {
        RenderData(const fonts::Symbol &symbol_);

        unsigned int id = 0;
        Vector2<int> size;
        int advance = 0;
    } renderData;

    FrameTimer<false> appearanceDelay;
    FrameTimer<false> appearanceDuration;

    const struct Effects
    {
        struct Shake
        {
            static const uint8_t defaultXAmp;
            static const uint8_t defaultYAmp;
            static const float defaultProb;

            // Amplitude, offset from -xAmp/2 to xAmp/2
            uint8_t xAmp = defaultXAmp;

            // Amplitude, offset from -yAmp/2 to yAmp/2
            uint8_t yAmp = defaultYAmp;

            // Probability of a given symbol being shaked by offset
            float prob = defaultProb;
        } shake;
    } effects;

    ChatSymbol(const fonts::Symbol &symbol_, CharAppearanceSpeed speed_, uint32_t extraDelay_, const Effects &effects_);
};

struct Line
{
    Line(int minx_, int height_);

    void addSymbol(ChatSymbol &&sym_);
    int height() const noexcept;
    int width() const noexcept;

    ChatSymbol &operator[](size_t id_);
    size_t size() const noexcept;
    std::vector<ChatSymbol> &symbols() noexcept;
    const std::vector<ChatSymbol> &symbols() const noexcept;

    const int minx = 0;
    
private:
    std::vector<ChatSymbol> m_symbols;
    const int m_height = 0;
    int m_width = 0;
};

/**
 *  Description of a text in a single text box, parsed in a constructor
 */
class ChatMessage
{
public:
    ChatMessage(const std::string &str_);

    bool update();
    void skip();

    Vector2<int> size() const noexcept;
    const std::vector<Line> &lines() const noexcept;

private:
    size_t m_lineForDelay = 0;
    size_t m_symbolForDelay = 0;
    std::vector<Line> m_lines;
    Vector2<int> m_size;
};

struct ChatMessageSequence
{
public:
    ChatMessageSequence(entt::entity source_, ChatBoxSide side_, bool fitScreen_);

    void addMessage(const std::string &message_ );
    bool empty() const noexcept;
    bool hasMessagesLeft() const noexcept;
    const ChatMessage &message() const noexcept;

    // Returns true if it's done and can be deleted
    bool update();

    // Called on input
    void proceed();

    ChatBoxSide side() const noexcept;

    entt::entity source() const;

    Vector2<int> currentSize() const noexcept;

    bool fitScreen() const noexcept;

private:
    size_t m_currentMessage = 0;
    std::vector<ChatMessage> m_messages;
    ChatBoxSide m_side;

    entt::entity m_source;

    // Timer used for box states
    FrameTimer<true> m_timer;

    enum class State : uint8_t 
    { 
        APPEAR, // Time before text starts appearing, uses timer naturally
        PRINTING, // Time while text is being printed, uses timer to resize it between replicas
        IDLE, // Time when text is complete, uses timer like PRINTING
        DISAPPEAR // Time after text disappeared, uses timer naturally
    } m_currentState = State::APPEAR;

    Vector2<int> lastSize;
    Vector2<int> newSize;

    bool m_fitScreen;
};

class SequenceRenderer
{
public:
    SequenceRenderer(const ChatMessageSequence &seq_, const entt::registry &reg_, const Camera &cam_, const Texture &chatboxEdge_, const Texture &chatboxPointer_);
    void draw();

private:
    void calcPos();
    void drawImpl() const;
    void drawMessageImpl(const ChatMessage &msg) const;

    Renderer &m_renderer;
    const ChatMessageSequence &m_seq;
    const entt::registry &m_reg;
    const Camera &m_cam;

    const Texture &m_chatboxEdge;
    const Texture &m_chatboxPointer;

    // Pixels along each axis from edges (their inner sides) to the actual sequence
    const int m_edgeGap = 4;

    // Including edges
    Vector2<int> m_internalSize;

    // Excluding edges
    Vector2<int> m_externalSize;

    Vector2<int> m_screenPos;
    Vector2<int> m_outerBoundTL, m_outerBoundBR;
    bool m_boxTop = false;
};

class ChatboxSystem : public InputReactor
{
public:
    ChatboxSystem(entt::registry &reg_, Camera &camera_, InputHandlingSystem &inputSystem_);
    
    void addSequence(ChatMessageSequence &&seq_);
    void update();
    void receiveEvents(HUD_EVENTS event, float scale_) override;
    void draw() const;

private:
    entt::registry &m_reg;
    Camera &m_camera;
    Renderer &m_renderer;
    InputHandlingSystem &m_inputSystem;
    std::vector<ChatMessageSequence> m_sequences;

    std::shared_ptr<Texture> m_chatboxEdge;
    std::shared_ptr<Texture> m_chatboxPointer;
};

