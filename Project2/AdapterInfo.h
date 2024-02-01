#pragma once

#include "EngineException.h"
#include <d3d11.h>
#pragma comment(lib,"d3d11.lib")
//#pragma comment(lib,"DirectXTK.lib")
#pragma comment(lib,"DXGI.lib")
#include <wrl/client.h>
#include <vector> 

// a pointer to an adapter and a description
class AdapterData
{
public:
	AdapterData(IDXGIAdapter* pAdapter);
	IDXGIAdapter* pAdapter = nullptr;
	DXGI_ADAPTER_DESC description;
};

class AdapterReader
{
public:
	// returns a vector of AdapterData
	static std::vector<AdapterData> GetAdapters();
private:
	static std::vector<AdapterData> adapters;
};


