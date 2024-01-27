#pragma once

#include <exception>
#include <string>
#include <sstream>

class EngineException : public std::exception
{
public:
	EngineException(int line, const char* file) noexcept;
	const char* what() const noexcept override;
	virtual const char* getType() const noexcept;
	int getLine() const noexcept;
	const std::string& getFile() const noexcept;
	std::string getOriginString() const noexcept;

private:
	int line;
	std::string file;

protected:
	mutable std::string infoBuffer;
};