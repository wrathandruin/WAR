#include <string>

#include <windows.h>

#include "engine/host/HeadlessHostBootstrap.h"

int wmain()
{
    return war::runHeadlessHostFromCommandLine(GetCommandLineW());
}
