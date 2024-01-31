#pragma once

#include "MinimalWin.h"
//#include <Windows.h>

#include "EngineException.h"
//#include "COMException.h"
#include <d3d11.h>
#pragma comment(lib,"d3d11.lib")
//#pragma comment(lib,"DirectXTK.lib")
#pragma comment(lib,"DXGI.lib")
#include <wrl/client.h>
#include <vector> 


class Graphics
{
public :
	
public:
	Graphics();
	~Graphics();
	bool Init(HWND hWnd, int width, int height);
	void EndFrame();
	void ClearBuffer(float red, float green, float blue) noexcept;
	//static void errLog(COMException& exception);
private:
	int width = 0;
	int height = 0;
	HWND hWnd = nullptr;

	ID3D11Device* deviceP = nullptr;
	ID3D11DeviceContext* deviceContextP = nullptr;
	IDXGISwapChain* swapchainP = nullptr;
	ID3D11RenderTargetView* targetP = nullptr;
	/*
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapchain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
	*/
};