//#include <sstream>
#include "EngineException.h"

EngineException::EngineException(int line, const char* file) noexcept
	: line(line), file(file)
{

}

const char* EngineException::what() const noexcept
{
	std::ostringstream osst;
	osst << getType() << std::endl << getOriginString();
	infoBuffer = osst.str();
	return infoBuffer.c_str();

}

const char* EngineException::getType() const noexcept
{
	return "Engine Exception";
}


int EngineException::getLine() const noexcept
{
	return line;
}

const std::string& EngineException::getFile() const noexcept
{
	return file;
}

std::string EngineException::getOriginString() const noexcept
{
	std::ostringstream osst;
	osst << "[File]  " << file << std::endl << "[Line]  " << line;
	return osst.str();
}