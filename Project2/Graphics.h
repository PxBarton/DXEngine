#pragma once

#include "MinimalWin.h"
//#include <Windows.h>

#include "EngineException.h"
#include "Shaders.h"
#include "AdapterInfo.h"
#include "Vertex.h"

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
	void RenderFrame();
	void ClearBuffer(float red, float green, float blue) noexcept;
	//ID3D11InputLayout* GetInputLayout();
	void BasicTri();

	//static void errLog(COMException& exception);
private:
	int width = 0;
	int height = 0;
	HWND hWnd = nullptr;
	VertexShader vertexShader;
	PixelShader pixelShader;

	Microsoft::WRL::ComPtr<ID3D11Device> deviceP;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContextP;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapchainP;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetViewP;
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;

	//Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayoutP;

	bool InitShaders();
	bool InitScene();
	
};