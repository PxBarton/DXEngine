#pragma once

#include "MinimalWin.h"
//#include <Windows.h>

#include <array>
#include <iostream>
#include <algorithm>

#include "EngineException.h"
#include "Shaders.h"
#include "Adapters.h"
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


class Renderer
{
public:

public:
	Renderer();
	~Renderer() = default;
	bool Init(HWND hWnd, int width, int height);
	void RenderFrame();
	Camera camera;
	XMFLOAT3 origin = XMFLOAT3(0.0, 0.0, 0.0);

	std::unique_ptr<Mesh> plane = nullptr;
	std::unique_ptr<Mesh> cube = nullptr;
	std::unique_ptr<Mesh> animatedPlane = nullptr;
	std::unique_ptr<Mesh> cylinder = nullptr;
	std::unique_ptr<Mesh> axisGrid = nullptr;
	std::unique_ptr<MeshSystem> cylinderSystem;
	std::unique_ptr<Mesh> flatPlane = nullptr;
	std::unique_ptr<Mesh> building = nullptr;
	

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
	ConstantBuffer<CB_vertex> cb_vert;
	ConstantBuffer<CB_light> cb_light;

	bool InitShaders();
	bool SceneSetup();
	void RenderSetup();
};