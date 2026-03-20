#pragma once

namespace war
{
    class Timer
    {
    public:
        void reset();
        float tick();

    private:
        long long m_frequency = 0;
        long long m_lastCounter = 0;
    };
}
