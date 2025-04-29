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

    hWnd = CreateWindowEx(
        // Optional window styles.
        0,
        getWndClass(),
        L"Basic BS",
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
    if (hWnd != NULL)
    {
        UnregisterClass(getWndClass(), hInst);
        DestroyWindow(hWnd);
    }
    
    
}

void EngineWindow::setTitle(const std::wstring& title)
{
    SetWindowText(hWnd, title.c_str());

}

const WCHAR* EngineWindow::getWndClass()
{
    return windowClass;
}

bool EngineWindow::InitEngine()
{
    static bool raw_input_initialized = false;
    if (raw_input_initialized == false)
    {
        RAWINPUTDEVICE rid;

        rid.usUsagePage = 0x01; //Mouse
        rid.usUsage = 0x02;
        rid.dwFlags = 0;
        rid.hwndTarget = NULL;

        if (RegisterRawInputDevices(&rid, 1, sizeof(rid)) == FALSE)
        {
            EngineException::Log(GetLastError(), "Failed to register raw input devices.");
            exit(-1);
        }

        raw_input_initialized = true;
    }

    if (!rndr->Init(hWnd, width, height))
    {
        return false;
    }
    return true;
}

bool EngineWindow::MessageProc()
{
    // handle windows messages
    MSG msg;
    ZeroMemory(&msg, sizeof(MSG));  // initialize message structure
    // location, handle, filters, remove
    // PeekMessage returns immediately even when no message
    while (PeekMessage(&msg, hWnd, 0, 0, PM_REMOVE))

    {
        TranslateMessage(&msg); // virtual key messages to character messages
        DispatchMessage(&msg);
    }
    
    if (msg.message == WM_NULL)
    {
        if (!IsWindow(hWnd))
        {
            hWnd = nullptr;
            UnregisterClass(getWndClass(), hInst);
            return false;
        }
    }
    
    return true;
}

void EngineWindow::Update()
{
    float time = timer.GetMilisecondsElapsed();
    timer.Restart();

    while (!keyboard.CharBufferIsEmpty())
    {
        unsigned char ch = keyboard.ReadChar();
    }

    while (!keyboard.KeyBufferIsEmpty())
    {
        KeyboardEvent kbe = keyboard.ReadKey();
        unsigned char keycode = kbe.GetKeyCode();
    }

    while (!mouse.EventBufferIsEmpty())
    {
        MouseEvent me = mouse.ReadEvent();
        if (mouse.IsRightDown())
        {
            if (me.GetType() == MouseEvent::EventType::RAW_MOVE)
            {
                this->rndr->camera.AdjustRotation((float)me.GetPosY() * 0.01f, (float)me.GetPosX() * 0.01f, 0);
            }
        }
    }

    const float cameraSpeed = 0.006f;
    float dt = 3.0;
    // move forward, back, left, right
    if (keyboard.KeyIsPressed('Y'))
    {
        this->rndr->camera.AdjustPosition(this->rndr->camera.GetForwardV() * cameraSpeed * time);
    }
    if (keyboard.KeyIsPressed('B'))
    {
        this->rndr->camera.AdjustPosition(this->rndr->camera.GetBackwardV() * cameraSpeed * time);
    }
    if (keyboard.KeyIsPressed('G'))
    {
        this->rndr->camera.AdjustPosition(this->rndr->camera.GetLeftV() * cameraSpeed * time);
    }
    if (keyboard.KeyIsPressed('H'))
    {
        this->rndr->camera.AdjustPosition(this->rndr->camera.GetRightV() * cameraSpeed * time);
    }

    // up, down
    if (keyboard.KeyIsPressed('R'))
    {
        this->rndr->camera.AdjustPosition(0.0f, 1.0f * cameraSpeed * dt, 0.0f);
    }
    if (keyboard.KeyIsPressed('C'))
    {
        this->rndr->camera.AdjustPosition(0.0f, -1.0f * cameraSpeed * dt, 0.0f);
    }

    // rotation
    if (keyboard.KeyIsPressed('W'))
    {
        this->rndr->camera.AdjustRotation(-0.01f, 0.0f, 0.0f);
    }
    if (keyboard.KeyIsPressed('Z'))
    {
        this->rndr->camera.AdjustRotation(0.01f, 0.0f, 0.0f);
    }
    if (keyboard.KeyIsPressed('A'))
    {
        this->rndr->camera.AdjustRotation(0.0f, -0.01f, 0.00f);
    }
    if (keyboard.KeyIsPressed('S'))
    {
        this->rndr->camera.AdjustRotation(0.0f, 0.01f, -0.00f);
    }

    // look at origin
    if (keyboard.KeyIsPressed('O'))
    {
        this->rndr->camera.SetLookAtPos(this->rndr->camera.origin);
    }
    // rotation about origin
    if (keyboard.KeyIsPressed(VK_UP))
    {
        this->rndr->camera.AdjustPosition(this->rndr->camera.GetForwardV() * cameraSpeed * 5 * time);
    }
    if (keyboard.KeyIsPressed(VK_DOWN))
    {
        this->rndr->camera.AdjustPosition(this->rndr->camera.GetBackwardV() * cameraSpeed * 5 * time);
    }
    if (keyboard.KeyIsPressed(VK_LEFT))
    {
        this->rndr->camera.AdjustPosition(this->rndr->camera.GetLeftV() * cameraSpeed * time);
        this->rndr->camera.SetLookAtPos(this->rndr->camera.origin);
    }
    if (keyboard.KeyIsPressed(VK_RIGHT))
    {
        this->rndr->camera.AdjustPosition(this->rndr->camera.GetRightV() * cameraSpeed * time);
        this->rndr->camera.SetLookAtPos(this->rndr->camera.origin);
    }
    
}

LRESULT EngineWindow::MsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
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

LRESULT EngineWindow::MsgHelper(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    EngineWindow* const pWnd = reinterpret_cast<EngineWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    return pWnd->HandleMsg(hwnd, msg, wParam, lParam);

}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT EngineWindow::HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
        //Keyboard Messages
    case WM_KEYDOWN:
    {
        unsigned char keycode = static_cast<unsigned char>(wParam);
        if (keyboard.IsKeysAutoRepeat())
        {
            keyboard.OnKeyPressed(keycode);
        }
        else
        {
            const bool wasPressed = lParam & 0x40000000;
            if (!wasPressed)
            {
                keyboard.OnKeyPressed(keycode);
            }
        }
        return 0;
    }
    case WM_KEYUP:
    {
        unsigned char keycode = static_cast<unsigned char>(wParam);
        keyboard.OnKeyReleased(keycode);
        return 0;
    }
    case WM_CHAR:
    {
        unsigned char ch = static_cast<unsigned char>(wParam);
        if (keyboard.IsCharsAutoRepeat())
        {
            keyboard.OnChar(ch);
        }
        else
        {
            const bool wasPressed = lParam & 0x40000000;
            if (!wasPressed)
            {
                keyboard.OnChar(ch);
            }
        }
        return 0;
    }

    //Mouse Messages
    case WM_MOUSEMOVE:
    {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        mouse.OnMouseMove(x, y);
        return 0;
    }
    case WM_LBUTTONDOWN:
    {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        mouse.OnLeftPressed(x, y);
        return 0;
    }
    case WM_RBUTTONDOWN:
    {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        mouse.OnRightPressed(x, y);
        return 0;
    }
    case WM_MBUTTONDOWN:
    {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        mouse.OnMiddlePressed(x, y);
        return 0;
    }
    case WM_LBUTTONUP:
    {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        mouse.OnLeftReleased(x, y);
        return 0;
    }
    case WM_RBUTTONUP:
    {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        mouse.OnRightReleased(x, y);
        return 0;
    }
    case WM_MBUTTONUP:
    {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        mouse.OnMiddleReleased(x, y);
        return 0;
    }
    case WM_MOUSEWHEEL:
    {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        if (GET_WHEEL_DELTA_WPARAM(wParam) > 0)
        {
            mouse.OnWheelUp(x, y);
        }
        else if (GET_WHEEL_DELTA_WPARAM(wParam) < 0)
        {
            mouse.OnWheelDown(x, y);
        }
        return 0;
    }
    case WM_INPUT:
    {
        UINT dataSize;
        GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, NULL, &dataSize, sizeof(RAWINPUTHEADER)); //Need to populate data size first

        if (dataSize > 0)
        {
            std::unique_ptr<BYTE[]> rawdata = std::make_unique<BYTE[]>(dataSize);
            if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, rawdata.get(), &dataSize, sizeof(RAWINPUTHEADER)) == dataSize)
            {
                RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(rawdata.get());
                if (raw->header.dwType == RIM_TYPEMOUSE)
                {
                    mouse.OnMouseMoveRaw(raw->data.mouse.lLastX, raw->data.mouse.lLastY);
                }
            }
        }

        return DefWindowProc(hWnd, msg, wParam, lParam); //Need to call DefWindowProc for WM_INPUT messages
    }

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
