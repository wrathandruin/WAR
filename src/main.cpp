#include <windows.h>

#include "engine/core/Application.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    war::Application app;
    return app.run();
}
