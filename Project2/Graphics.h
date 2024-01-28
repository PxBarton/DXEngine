#pragma once

#include "MinimalWin.h"

#include <d3d11.h>
#pragma comment(lib,"d3d11.lib")
//#pragma comment(lib,"DirectXTK.lib")
#pragma comment(lib,"DXGI.lib")
#include <wrl/client.h>
#include <vector> 


class Graphics
{
public:
	Graphics();
	~Graphics();
	void Init(HWND hWnd);
	void EndFrame();

private:
	int width;
	int height;
	HWND hWnd = nullptr;

	ID3D11Device* deviceP = nullptr;
	ID3D11DeviceContext* deviceContextP = nullptr;
	IDXGISwapChain* swapchainP = nullptr;

	/*
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapchain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
	*/
};