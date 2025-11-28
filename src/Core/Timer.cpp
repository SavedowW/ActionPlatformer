#include "Timer.h"
#include "Application.h"
#include <SDL3/SDL.h>
#include <string>

namespace Time {

    std::ostream &operator<<(std::ostream &lhs_, const NS &rhs_)
    {
        return lhs_ << rhs_.m_time << "ns";
    }

    NS deserialize(const std::string &s_)
    {
        if (s_.size() >= 3 && s_.substr(s_.size() - 2) == "ns")
            return NS{std::stoull(s_.substr(0, s_.size() - 2))};

        return Application::TimeStep::defaultFrameDuration * std::stoull(s_);
    }

    NS fromFrames(uint64_t frame_)
    {
        return Application::TimeStep::defaultFrameDuration * frame_;
    }
}

void Timer::begin() noexcept
{
    m_timeBegin = Time::NS{SDL_GetTicksNS()};
}

Time::NS Timer::getPassedNS() const noexcept
{
    return Time::NS{SDL_GetTicksNS()} - m_timeBegin;
}

Time::NS Timer::iterateNS() noexcept
{
    const auto newTimeBegin = Time::NS{SDL_GetTicksNS()};
    const auto res = newTimeBegin - m_timeBegin;
    m_timeBegin = newTimeBegin;
    return res;
}
