#pragma once

#include <exception>
#include <string>
#include <sstream>
#include "COMException.h"

class EngineException : public std::exception
{
public:
	EngineException(int line, const char* file) noexcept;
	const char* what() const noexcept override;
	virtual const char* getType() const noexcept;
	int getLine() const noexcept;
	const std::string& getFile() const noexcept;
	std::string getOriginString() const noexcept;
	
	static void Log(std::string message)
	{
		std::string error_message = "Error: " + message;
		MessageBoxA(nullptr, error_message.c_str(), "Error", MB_ICONERROR);
	}
	static void Log(HRESULT hr, std::string message)
	{
		_com_error error(hr);
		std::wstring error_message = L"Error: " + COMException::wideStr(message) + L"\n" + error.ErrorMessage();
		MessageBoxW(nullptr, error_message.c_str(), L"Error", MB_ICONERROR);
	}

	// file path stored as a wide string
	static void Log(HRESULT hr, std::wstring message)
	{
		_com_error error(hr);
		std::wstring error_message = L"Error: " + message + L"\n" + error.ErrorMessage();
		MessageBoxW(nullptr, error_message.c_str(), L"Error", MB_ICONERROR);
	}

	static void Log(COMException& exception)
	{
		std::wstring error_message = exception.what();
		MessageBoxW(NULL, error_message.c_str(), L"Error", MB_ICONERROR);
	}

private:
	int line;
	std::string file;

protected:
	mutable std::string infoBuffer;
};