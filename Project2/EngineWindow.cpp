#include "EngineWindow.h"
#include <assert.h>

EngineWindow::WinClass EngineWindow::WinClass::wClass;

EngineWindow::WinClass::WinClass() noexcept : hInstance(GetModuleHandle(nullptr))
{
    WNDCLASSEX wc = { };

    wc.cbSize = sizeof(wc);
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = MsgSetup;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"fghashasiu";
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;

    RegisterClassEx(&wc);
}

EngineWindow::WinClass::~WinClass()
{
    UnregisterClass(winClassName, getInstance());
}

const WCHAR* EngineWindow::WinClass::getName() noexcept
{
    return winClassName;
}

HINSTANCE EngineWindow::WinClass::getInstance() noexcept
{
    return wClass.hInstance;
}

EngineWindow::EngineWindow(int w, int h, const char* t) noexcept : width(w), height(h)
{

    hwnd = CreateWindowEx(
        // Optional window styles.
        0,
        WinClass::getName(),
        L"fucking garbage",
        // Window style
        WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,

        // Size and position
        400, 100, width, height,

        nullptr,       // Parent window    
        nullptr,       // Menu
        WinClass::getInstance(),  // Instance handle
        this
    );        // Additional application data

    ShowWindow(hwnd, SW_SHOWDEFAULT);
}

EngineWindow::~EngineWindow()
{
    //ImGui_ImplWin32_Shutdown();
    DestroyWindow(hwnd);
}

void EngineWindow::setTitle(const std::wstring& title)
{
    SetWindowText(hwnd, title.c_str());

}


LRESULT CALLBACK EngineWindow::MsgSetup(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    // use create parameter passed in from CreateWindow() to store window class pointer at WinAPI side
    if (msg == WM_NCCREATE)
    {
        // extract ptr to window class from creation data
        const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lparam);
        EngineWindow* const pWnd = reinterpret_cast<EngineWindow*>(pCreate->lpCreateParams);
        // sanity check
        assert(pWnd != nullptr);
        // set WinAPI-managed user data to store ptr to window class
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
        // set message proc to normal (non-setup) handler now that setup is finished
        SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&EngineWindow::MsgHelper));
        // forward message to window class handler
        return pWnd->HandleMsg(hwnd, msg, wparam, lparam);
    }
    // if we get a message before the WM_NCCREATE message, handle with default handler
    return DefWindowProc(hwnd, msg, wparam, lparam);

}

LRESULT CALLBACK EngineWindow::MsgHelper(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    EngineWindow* const pWnd = reinterpret_cast<EngineWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    return pWnd->HandleMsg(hwnd, msg, wParam, lParam);

}

LRESULT EngineWindow::HandleMsg(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wparam, lparam);

}
