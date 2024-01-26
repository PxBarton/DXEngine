#pragma once

#include "MinimalWin.h"
#include <string>

class EngineWindow
{
private:
	class WinClass
	{
	public:
		static const WCHAR* getName() noexcept;
		static HINSTANCE getInstance() noexcept;
	private:
		WinClass() noexcept;
		~WinClass();
		WinClass(const WinClass&) = delete;
		WinClass& operator=(const WinClass&) = delete;
		static constexpr const WCHAR* winClassName = L"DX3d Engine Window";
		static WinClass wClass;
		HINSTANCE hInstance;

	};
public:
	EngineWindow(int w, int h, const char* t) noexcept;
	~EngineWindow();
	EngineWindow(const EngineWindow&) = delete;
	EngineWindow& operator=(const EngineWindow&) = delete;
	void setTitle(const std::wstring& title);
private:
	int width;
	int height;
	HWND hwnd;

	static LRESULT CALLBACK MsgSetup(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	static LRESULT CALLBACK MsgHelper(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMsg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};