#include "Graphics.h"


Graphics::Graphics() 
{}

bool Graphics::Init(HWND hWnd, int width, int height)
{
	this->width = width;
	this->height = height;

	try
	{
		DXGI_SWAP_CHAIN_DESC scd;
		ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

		scd.BufferDesc.Width = 0;
		scd.BufferDesc.Height = 0;
		scd.BufferDesc.RefreshRate.Numerator = 60;
		scd.BufferDesc.RefreshRate.Denominator = 1;
		scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		scd.SampleDesc.Count = 1;
		scd.SampleDesc.Quality = 0;

		scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scd.BufferCount = 1;
		scd.OutputWindow = hWnd;
		scd.Windowed = TRUE;
		scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		D3D11CreateDeviceAndSwapChain(nullptr,    // default is first adapter in list (Nvidia 1650) 
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			0,
			nullptr,
			0,
			D3D11_SDK_VERSION,
			&scd,    // swapchain description
			&swapchainP,
			&deviceP,
			nullptr,
			&deviceContextP);

		ID3D11Resource* backBufferP = nullptr;
		swapchainP->GetBuffer(0, __uuidof(ID3D11Resource), reinterpret_cast<void**>(&backBufferP));
		deviceP->CreateRenderTargetView(backBufferP, nullptr, &targetP);
		backBufferP->Release();
	}
	catch (COMException exception)
	{
		EngineException::Log(exception);
		return false;
	}

	return true;
}

Graphics::~Graphics()
{
	if (targetP != nullptr)
	{
		targetP->Release();
	}
	if (swapchainP != nullptr)
	{
		swapchainP->Release();
	}
	if (deviceContextP != nullptr)
	{
		deviceContextP->Release();
	}
	if (deviceP != nullptr)
	{
		deviceP->Release();
	}
}

void Graphics::EndFrame()
{
	swapchainP->Present(1u, 0u);
}

void Graphics::ClearBuffer(float red, float green, float blue) noexcept
{
	const float color[] = { red, green, blue, 1.0f };
	deviceContextP->ClearRenderTargetView(targetP, color);
}

/*
void Graphics::errLog(COMException& exception)
{
	std::wstring error_message = exception.what();
	MessageBoxW(NULL, error_message.c_str(), L"Error", MB_ICONERROR);
}
*/
