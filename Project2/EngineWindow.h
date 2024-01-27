#pragma once

#include "MinimalWin.h"
#include <string>

class EngineWindow
{

public:
	EngineWindow(int w, int h, const char* t) noexcept;
	~EngineWindow();
	EngineWindow(const EngineWindow&) = delete;
	EngineWindow& operator=(const EngineWindow&) = delete;
	void setTitle(const std::wstring& title);
private:
	int width;
	int height;
	HINSTANCE hInst = nullptr;
	HWND hWnd = nullptr;
	

	static LRESULT CALLBACK MsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK MsgHelper(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};