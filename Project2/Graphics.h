#pragma once

#include "MinimalWin.h"
//#include <Windows.h>

#include <array>
#include <iostream>
#include <algorithm>

#include "EngineException.h"
#include "Shaders.h"
#include "AdapterInfo.h"
#include "Vertex.h"
#include "Buffers.h"
#include "Mesh.h"
#include "MeshSystem.h"
#include "Camera.h"
#include <cmath>
#include <WICTextureLoader.h>
//#pragma comment(lib,"d3d11.lib")
//#pragma comment(lib,"DirectXTK.lib")
//#pragma comment(lib,"DXGI.lib")
//#include <wrl/client.h>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"


class Graphics
{
public:

public:
	Graphics();
	~Graphics() = default;
	bool Init(HWND hWnd, int width, int height);
	void RenderFrame();
	Camera camera;
	
	std::unique_ptr<Mesh> plane = nullptr;
	std::unique_ptr<Mesh> cube = nullptr;
	std::unique_ptr<Mesh> animatedPlane = nullptr;
	std::unique_ptr<Mesh> cylinder = nullptr;
	std::unique_ptr<MeshSystem> cubeSystem = nullptr;
	std::unique_ptr<MeshSystem> cubeSystem2 = nullptr;
	std::unique_ptr<MeshSystem> cubeSystem3 = nullptr;
	std::unique_ptr<Mesh> cylinder1 = nullptr;
	std::unique_ptr<Mesh> cylinder2 = nullptr;
	std::shared_ptr<Mesh> cylinder8 = nullptr;

	int numMeshes = 4000;
	std::vector<InstancePosition> instData{ numMeshes };
	std::unique_ptr<InstancePosition[]> instanceData = nullptr;

	void setNumMeshes(int num)
	{
		numMeshes = num;
	}

private:
	int width = 0;
	int height = 0;
	HWND hWnd = nullptr;
	VertexShader vertexShader;
	PixelShader pixelShader;
	std::vector<XMFLOAT3> triNormals;

	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapchain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;
	Microsoft::WRL::ComPtr<ID3D11BlendState> blendState;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture;

	VertexBuffer<Vertex> vertexBuffer;
	IndexBuffer indicesBuffer;
	InstanceBuffer<InstancePosition> instanceBuffer;
	ConstantBuffer<CB_VS_vertexshader> cb_vert;
	ConstantBuffer<CB_PS_light> cb_light;

	//Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;

	//Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayoutP;

	bool InitShaders();
	//bool InitScene(Vertex v[], DWORD i[], UINT lenV, UINT lenI);
	bool InitScene();
	// C++ STL <vector> version
	//bool InitScene(std::vector<Vertex> v, std::vector<DWORD> i, UINT lenV, UINT lenI);

};