#pragma once

#include <string>
#include "MinimalWin.h"
#include "EngineException.h"

class EngineWindow
{
public:
	class Expt : public EngineException
	{
	public:
		Expt(int line, const char* file, HRESULT hr) noexcept;
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
	const WCHAR* getWndClass();
	bool MessageProc();

private:
	int width;
	int height;
	const WCHAR* windowClass = L"0";
	HINSTANCE hInst = nullptr;
	HWND hWnd = nullptr;
	
	static LRESULT CALLBACK MsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK MsgHelper(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};


// exception macro
#define ENG_EXCEPT(hr) EngineWindow::Expt(__LINE__, __FILE__, hr)