#ifndef UNICODE
#define UNICODE
#endif 

#include "EngineWindow.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    try
    {
        EngineWindow wind(1000, 800, "trash");
        if (wind.InitGfx()) {
            while (wind.MessageProc() == true)
            {
                wind.Update();
                wind.gfx.RenderFrame();
            }
        }
        


        /*
        MSG msg = { };

        while (GetMessage(&msg, nullptr, 0, 0) > 0)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        */

        return 0;
    }
    catch (const EngineException& e)
    {
        MessageBoxA(nullptr, e.what(), e.getType(), MB_OK | MB_ICONEXCLAMATION);
    }
    catch (const std::exception& e)
    {
        MessageBoxA(nullptr, e.what(), "Std Exception", MB_OK | MB_ICONEXCLAMATION);
    }
    catch (...)
    {
        MessageBoxA(nullptr, "No Details Available", "Unknown Exception", MB_OK | MB_ICONEXCLAMATION);
    }
    return -1;
}
