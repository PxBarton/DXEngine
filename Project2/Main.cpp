#ifndef UNICODE
#define UNICODE
#endif 

#include "EngineWindow.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    try
    {
        EngineWindow wind(1200, 900, "trash");
        if (wind.InitEngine()) {
            while (wind.MessageProc() == true)
            {
                wind.Update();
                wind.rndr->RenderFrame();
            }
        }
        
        return 0;
    }
    catch (const EngineException& e)
    {
        MessageBoxA(nullptr, e.what(), e.getType(), MB_OK | MB_ICONEXCLAMATION);
    }
    return -1;
}
