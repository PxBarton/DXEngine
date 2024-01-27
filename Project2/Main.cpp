#ifndef UNICODE
#define UNICODE
#endif 

#include "EngineWindow.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    EngineWindow wind(400, 400, "trash");
    EngineWindow wind2(500, 300, "junk");
    

    MSG msg = { };
    
    while (GetMessage(&msg, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    
    return 0;
}
