#include "Adapters.h"

std::vector<AdapterData> AdapterReader::adapters = {};

std::vector<AdapterData> AdapterReader::GetAdapters()
{
	if (adapters.size() > 0) //If already initialized
		return adapters;

	Microsoft::WRL::ComPtr<IDXGIFactory> factory;

	// Create a DXGIFactory object.
	HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(factory.GetAddressOf()));
	if (FAILED(hr))
	{
		EngineException::Log(hr, "Failed to create DXGIFactory for enumerating adapters.");
		exit(-1);
	}

	IDXGIAdapter* adapter;
	UINT index = 0;
	while (SUCCEEDED(factory->EnumAdapters(index, &adapter)))
	{
		adapters.push_back(AdapterData(adapter));
		index += 1;
	}
	return adapters;
}

AdapterData::AdapterData(IDXGIAdapter* adapter)
{
	this->adapter = adapter;
	HRESULT hr = adapter->GetDesc(&this->description);
	if (FAILED(hr))
	{
		EngineException::Log(hr, "Failed to Get Description for IDXGIAdapter.");
	}
}