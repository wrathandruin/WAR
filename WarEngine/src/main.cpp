#include <string>
#include <windows.h>

#include "engine/core/Application.h"
#include "engine/host/HeadlessHostBootstrap.h"

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR commandLine, int)
{
    const std::wstring commandLineText = commandLine != nullptr ? std::wstring(commandLine) : std::wstring{};
    if (commandLineText.find(L"--headless-host") != std::wstring::npos)
    {
        return war::runHeadlessHostFromCommandLine(commandLineText);
    }

    war::Application app;
    return app.run(commandLineText);
}
