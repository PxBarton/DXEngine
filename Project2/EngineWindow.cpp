#include "EngineWindow.h"
#include <assert.h>




EngineWindow::EngineWindow(int w, int h, const char* t) noexcept
    : width(w), height(h), hInst(GetModuleHandle(nullptr))

{
    const wchar_t* CLASS_NAME = L"DXEngineWinClass";

    WNDCLASSEX wc = { };

    wc.cbSize = sizeof(wc);
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = MsgSetup;
    wc.hInstance = hInst;
    wc.lpszClassName = CLASS_NAME;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;


    RegisterClassEx(&wc);
    /*
    if (!RegisterClassEx(&wc))
    {
        MessageBox(nullptr,
            L"Call to RegisterClassEx failed!",
            L"Windows Desktop Guided Tour",
            0);

        //return 1;
    }
    */ 

    hWnd = CreateWindowEx(
        // Optional window styles.
        0,
        CLASS_NAME,
        L"fucking garbage",
        // Window style
        WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,

        // Size and position
        400, 100, width, height,

        nullptr,       // Parent window    
        nullptr,       // Menu
        hInst,  // Instance handle
        this
    );        // Additional application data

    ShowWindow(hWnd, SW_SHOWDEFAULT);
}

EngineWindow::~EngineWindow()
{
    //ImGui_ImplWin32_Shutdown();
    DestroyWindow(hWnd);
}

void EngineWindow::setTitle(const std::wstring& title)
{
    SetWindowText(hWnd, title.c_str());

}


LRESULT CALLBACK EngineWindow::MsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // use create parameter passed in from CreateWindow() to store window class pointer at WinAPI side
    if (msg == WM_NCCREATE)
    {
        // extract ptr to window class from creation data
        const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
        EngineWindow* const pWnd = reinterpret_cast<EngineWindow*>(pCreate->lpCreateParams);
        // sanity check
        assert(pWnd != nullptr);
        // set WinAPI-managed user data to store ptr to window class
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
        // set message proc to normal (non-setup) handler now that setup is finished
        SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&EngineWindow::MsgHelper));
        // forward message to window class handler
        return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
    }
    // if we get a message before the WM_NCCREATE message, handle with default handler
    return DefWindowProc(hWnd, msg, wParam, lParam);

}

LRESULT CALLBACK EngineWindow::MsgHelper(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    EngineWindow* const pWnd = reinterpret_cast<EngineWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    return pWnd->HandleMsg(hwnd, msg, wParam, lParam);

}

LRESULT CALLBACK EngineWindow::HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);

}
