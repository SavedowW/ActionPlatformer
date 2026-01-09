#pragma once
#include <cstdint>
#include <array>

/**
 *  Class responsible for keeping consistent FPS 
 *  cycle method should be called every frame
 */
class FPSUtility
{
public:
    FPSUtility(uint64_t targettedFPS_);

    void setFPS(uint64_t targettedFPS_);
    void setDefaultFPS();

    void start();
    
    /**
     *  Sleep if it's required to keep consistent framerate
     */
    void cycle();

    // Debug only
    std::array<uint64_t, 2> lastCycleCalls;
    
private:
    void updateSyncPoint();

    inline constexpr static uint64_t s_syncLimit = 5;
    uint64_t m_lastSyncPointNS = 0;
    uint64_t m_properFrameDurationNS = 0;
    const uint64_t m_defaultProperFrameDurationNS = 0;
};
