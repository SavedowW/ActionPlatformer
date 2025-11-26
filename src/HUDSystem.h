#ifndef HUD_SYSTEM_H_
#define HUD_SYSTEM_H_
#include "Core/FixedQueue.hpp"
#include "Core/Texture.h"
#include "CommonAI.h"
#include <entt/entt.hpp>

template<Numeric T, size_t len, uint8_t updatePeriod>
class AveragingQueue : public FixedQueue<T, len>
{
public:
    void push(const T &val_) override
    {
        auto &current = FixedQueue<T, len>::m_data[FixedQueue<T, len>::m_nextToFill];

        if (FixedQueue<T, len>::m_filled >= len)
        {
            m_sum -= current;
            m_sum += val_;
        }
        else
        {
            m_sum += val_;
            FixedQueue<T, len>::m_filled++;
        }

        current = val_;

        FixedQueue<T, len>::m_nextToFill = (FixedQueue<T, len>::m_nextToFill + 1) % len;

        if (++m_iter % updatePeriod == 0)
            m_lastAvg = m_sum / FixedQueue<T, len>::m_filled;
    }

    T avg() const noexcept
    {
        return m_lastAvg;
    }

    virtual ~AveragingQueue() = default;

protected:
    uint8_t m_iter = 0;
    T m_lastAvg = 0;
    T m_sum = 0;
};

struct HudSystem
{
public:
    HudSystem(entt::registry &reg_, Camera &cam_, int lvlId_, const Vector2<float> &lvlSize_, const uint64_t &frameTime_);

    void draw() const;
    void drawCommonDebug() const;
    void drawPlayerDebug() const;
    void drawNPCDebug(const ComponentTransform &trans_, const ComponentPhysical &phys_, const StateMachine &sm_, const ComponentAI &ai_) const;

    entt::entity playerId = entt::null;

private:
    Renderer &m_renderer;
    TextManager &m_textManager;
    entt::registry &m_reg;

    Camera &m_cam;
    int m_lvlId;
    Vector2<float> m_lvlSize;
    const  uint64_t &m_frameTime;
    mutable AveragingQueue<float, 20, 5> m_avgFrames;

    std::shared_ptr<Texture> m_arrowIn;
    std::shared_ptr<Texture> m_arrowOut;
};

#endif
