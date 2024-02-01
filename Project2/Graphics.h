#pragma once

#include "MinimalWin.h"
//#include <Windows.h>

#include "EngineException.h"
#include "HLSLShaders.h"
#include "AdapterInfo.h"

//#pragma comment(lib,"d3d11.lib")
//#pragma comment(lib,"DirectXTK.lib")
//#pragma comment(lib,"DXGI.lib")
//#include <wrl/client.h>
//#include <vector> 


class Graphics
{
public :
	
public:
	Graphics();
	~Graphics() = default;
	bool Init(HWND hWnd, int width, int height);
	bool InitShaders();
	void RenderFrame();
	void ClearBuffer(float red, float green, float blue) noexcept;
	void BasicTri();

	//static void errLog(COMException& exception);
private:
	int width = 0;
	int height = 0;
	HWND hWnd = nullptr;
	VertexShader vertexShader;

	//ID3D11Device* deviceP = nullptr;
	//ID3D11DeviceContext* deviceContextP = nullptr;
	//IDXGISwapChain* swapchainP = nullptr;
	//ID3D11RenderTargetView* targetP = nullptr;
	
	Microsoft::WRL::ComPtr<ID3D11Device> deviceP;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContextP;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapchainP;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetViewP;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayoutP;
	
};