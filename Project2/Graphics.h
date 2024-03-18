#pragma once

#include "MinimalWin.h"
//#include <Windows.h>

#include "EngineException.h"
#include "Shaders.h"
#include "AdapterInfo.h"
#include "Vertex.h"
#include "Buffers.h"
#include "Mesh.h"
#include "Camera.h"

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
	
private:
	int width = 0;
	int height = 0;
	HWND hWnd = nullptr;
	VertexShader vertexShader;
	PixelShader pixelShader;
	Camera camera;

	Microsoft::WRL::ComPtr<ID3D11Device> deviceP;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContextP;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapchainP;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetViewP;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;

	VertexBuffer<Vertex> vertexBuffer;
	IndexBuffer indicesBuffer;
	ConstantBuffer<CB_VS_vertexshader> constBuffer;
	//Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;

	//Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayoutP;

	bool InitShaders();
	bool InitScene(Vertex v[], DWORD i[], UINT lenV, UINT lenI);
	// C++ STL <vector> version
	//bool InitScene(std::vector<Vertex> v, std::vector<DWORD> i, UINT lenV, UINT lenI);
	bool buildShape();
	
	DirectX::XMFLOAT3 triNormal(Vertex &A, Vertex& B, Vertex& C);
	// XMVECTOR version
	DirectX::XMVECTOR triNormalV(Vertex& A, Vertex& B, Vertex& C);
	void calcNormals(Vertex verts[], DWORD tris[]);
	// XMVECTOR version
	void calcNormalsV(Vertex verts[], DWORD tris[]);

	//DirectX::XMFLOAT3 vertNormal(Vertex verts[], DWORD tris[]);

};