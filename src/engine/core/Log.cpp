#include "engine/core/Log.h"

#include <windows.h>

#include <string>

namespace war
{
    void Log::info(std::string_view message)
    {
        std::string text = "[INFO] ";
        text += message;
        text += "\n";
        OutputDebugStringA(text.c_str());
    }
}
