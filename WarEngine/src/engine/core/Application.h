#pragma once

#include <string>

namespace war
{
    class Application
    {
    public:
        [[nodiscard]] int run(const std::wstring& commandLine);
    };
}
