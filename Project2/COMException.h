#pragma once

#include <comdef.h>
#include <string>

#define IF_COM_FAIL( hr, msg ) if( FAILED( hr ) ) throw COMException( hr, msg, __FILE__, __FUNCTION__, __LINE__ )

class COMException
{
public:
	COMException(HRESULT hr, const std::string& msg, const std::string& file, const std::string& function, int line)
	{
		_com_error error(hr);
		whatmsg = L"Msg: " + wideStr(std::string(msg)) + L"\n";
		whatmsg += error.ErrorMessage();
		whatmsg += L"\nFile: " + wideStr(file);
		whatmsg += L"\nFunction: " + wideStr(function);
		whatmsg += L"\nLine: " + wideStr(std::to_string(line));
	}

	const wchar_t* what() const
	{
		return whatmsg.c_str();
	}

	static std::wstring wideStr(std::string str)
	{
		std::wstring wide_string(str.begin(), str.end());
		return wide_string;
	}
private:
	std::wstring whatmsg;
};