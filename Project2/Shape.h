#pragma once

#include "Vertex.h"
#include "EngineException.h"

#include <vector>

struct Shape
{
	std::vector<Vertex> vertList;
	std::vector<DWORD> indexList;

};
