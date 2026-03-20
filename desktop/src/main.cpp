#include <string>

#include <windows.h>

#include "engine/core/Application.h"

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR commandLine, int)
{
    war::Application app;
    return app.run(commandLine != nullptr ? std::wstring(commandLine) : std::wstring{});
}
