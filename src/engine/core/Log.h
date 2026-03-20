#pragma once

#include <string_view>

namespace war
{
    struct Log
    {
        static void info(std::string_view message);
    };
}
