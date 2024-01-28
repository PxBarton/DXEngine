#include "EngineWindow.h"
#include <assert.h>




EngineWindow::EngineWindow(int w, int h, const char* t) noexcept
    : width(w), height(h), hInst(GetModuleHandle(nullptr))

{
    //setTitle(t);
    windowClass = L"DXEngineWinClass";
    WNDCLASSEX wc = { };

    wc.cbSize = sizeof(wc);
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = MsgSetup;
    wc.hInstance = hInst;
    wc.lpszClassName = getWndClass();
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
        getWndClass(),
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
    UnregisterClass(getWndClass(), hInst);
}

void EngineWindow::setTitle(const std::wstring& title)
{
    SetWindowText(hWnd, title.c_str());

}

const WCHAR* EngineWindow::getWndClass()
{
    return windowClass;
}

bool EngineWindow::MessageProc()
{
    // handle windows messages
    MSG msg;
    ZeroMemory(&msg, sizeof(MSG));  // initialize message structure
    // location, handle, filters, remove
    if (PeekMessage(&msg, hWnd, 0, 0, PM_REMOVE))

    {
        TranslateMessage(&msg); // virtual key messages to character messages
        DispatchMessage(&msg);
    }
    /*
    if (msg.message == WM_NULL)
    {
        if (!IsWindow(hWnd))
        {
            hWnd = nullptr;
            UnregisterClass(getWndClass(), hInst);
            return false;
        }
    }
    */
    return true;
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

///////////////////////
//    Exceptions

EngineWindow::WinExcept::WinExcept(int line, const char* file, HRESULT hr) noexcept
    : EngineException(line, file), hr(hr)
{
    
}

const char* EngineWindow::WinExcept::what() const noexcept
{
    std::ostringstream osst;
    osst << getType() << std::endl
        << "[Error Code]  " << getErrorCode() << std::endl
        << "[Description]  " << getErrorString() << std::endl
        << getOriginString();
    infoBuffer = osst.str();
    return infoBuffer.c_str();

}

const char* EngineWindow::WinExcept::getType() const noexcept
{
    return "Engine Window Exception";
}

std::string EngineWindow::WinExcept::translateErrorCode(HRESULT hr)
{
    char* pMsgBuf = nullptr;
    // windows will allocate memory for err string and make our pointer point to it
    const DWORD nMsgLen = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPWSTR>(&pMsgBuf), 0, nullptr
    );
    // 0 string length returned indicates a failure
    if (nMsgLen == 0)
    {
        return "Unidentified error code";
    }
    // copy error string from windows-allocated buffer to std::string
    std::string errorString = pMsgBuf;
    // free windows buffer
    LocalFree(pMsgBuf);
    return errorString;
}

HRESULT EngineWindow::WinExcept::getErrorCode() const noexcept
{
    return hr;
}

std::string EngineWindow::WinExcept::getErrorString() const noexcept
{
    return translateErrorCode(hr);
}
