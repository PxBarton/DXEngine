#pragma once

#include "MinimalWin.h"
//#include <Windows.h>
#include "Keyboard.h"
#include "Mouse.h"
#include "Graphics.h"

#include <string>
#include <SDKDDKVer.h>
//#include "Utilities.h"

class EngineWindow
{
public:
	class WinExcept : public EngineException
	{
	public:
		WinExcept(int line, const char* file, HRESULT hr) noexcept;
		const char* what() const noexcept override;
		virtual const char* getType() const noexcept override;
		static std::string translateErrorCode(HRESULT hr);
		HRESULT getErrorCode() const noexcept;
		std::string getErrorString() const noexcept;
	private:
		HRESULT hr;
	};

public:
	EngineWindow(int w, int h, const char* t) noexcept;
	~EngineWindow();
	EngineWindow(const EngineWindow&) = delete;
	EngineWindow& operator=(const EngineWindow&) = delete;
	void setTitle(const std::wstring& title);
	bool InitGfx();
	const WCHAR* getWndClass();
	bool MessageProc();
	void Update();
	//Graphics gfx;
	std::unique_ptr<Graphics> gfx = std::make_unique<Graphics>();
	
private:
	int width;
	int height;
	const WCHAR* windowClass = L"0";
	HINSTANCE hInst = nullptr;
	HWND hWnd = nullptr;
	Keyboard keyboard;
	Mouse mouse;
	Timer timer;
	
	static LRESULT MsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static LRESULT MsgHelper(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};


// exception macro
#define ENG_EXCEPT(hr) EngineWindow::WinExcept(__LINE__, __FILE__, hr)