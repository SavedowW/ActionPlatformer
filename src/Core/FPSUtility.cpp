#include "FPSUtility.h"
#include <SDL3/SDL.h>
#include <iostream>

FPSUtility::FPSUtility(uint64_t targettedFPS_):
    m_defaultProperFrameDurationNS{1'000'000'000ull / targettedFPS_}
{
    setDefaultFPS();
}

void FPSUtility::setFPS(uint64_t targettedFPS_)
{
    m_properFrameDurationNS = 1'000'000'000ull / targettedFPS_;
}

void FPSUtility::setDefaultFPS()
{
    m_properFrameDurationNS = m_defaultProperFrameDurationNS;
}

void FPSUtility::start()
{
    updateSyncPoint();
}

void FPSUtility::updateSyncPoint()
{
    m_lastSyncPointNS = SDL_GetTicksNS();
}

void FPSUtility::cycle()
{
    const uint64_t currentTS = SDL_GetTicksNS();
    const auto frameDur = currentTS - m_lastSyncPointNS;

    // Debug only
    lastCycleCalls[1] = lastCycleCalls[0];
    lastCycleCalls[0] = currentTS;

    if (frameDur <= m_properFrameDurationNS) // Frame was faster or exactly as necessary
    {
        // Just wait until we are close enough
        
        m_lastSyncPointNS += m_properFrameDurationNS;
        SDL_DelayPrecise(m_properFrameDurationNS - frameDur);
    }
    else
    {
        const uint64_t passedFrames = frameDur / m_properFrameDurationNS;

        // If there were too many slow frames - recovery attempt may cause long speedup, so let's try to at least recover regular speed
        if (passedFrames >= s_syncLimit)
        {
            std::cout << "Forced to skip " << passedFrames << " frames" << std::endl;
            updateSyncPoint();
        }
    }
}
