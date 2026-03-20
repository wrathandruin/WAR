#include "engine/core/Timer.h"

#include <windows.h>

namespace war
{
    void Timer::reset()
    {
        LARGE_INTEGER frequency{};
        LARGE_INTEGER counter{};
        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&counter);

        m_frequency = frequency.QuadPart;
        m_lastCounter = counter.QuadPart;
    }

    float Timer::tick()
    {
        LARGE_INTEGER counter{};
        QueryPerformanceCounter(&counter);

        const long long delta = counter.QuadPart - m_lastCounter;
        m_lastCounter = counter.QuadPart;

        if (m_frequency <= 0)
        {
            return 0.016f;
        }

        return static_cast<float>(static_cast<double>(delta) / static_cast<double>(m_frequency));
    }
}
