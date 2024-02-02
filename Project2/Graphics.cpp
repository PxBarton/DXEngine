#include "Graphics.h"

namespace wrl = Microsoft::WRL;

Graphics::Graphics() 
{}

bool Graphics::Init(HWND hWnd, int width, int height)
{
	this->hWnd = hWnd;
	this->width = width;
	this->height = height;

	std::vector<AdapterData> adapters = AdapterReader::GetAdapters();

	if (adapters.size() < 1)
	{
		EngineException::Log("No IDXGI Adapters.");
		return false;
	}

	try
	{
		DXGI_SWAP_CHAIN_DESC scd = { 0 };
		//ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

		scd.BufferDesc.Width = width;
		scd.BufferDesc.Height = height;
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

		D3D11CreateDeviceAndSwapChain(
			//NULL,    // default is first adapter in list (Nvidia 1650) 
			adapters[0].pAdapter,
			D3D_DRIVER_TYPE_UNKNOWN,
			NULL,
			NULL,
			NULL,
			0,
			D3D11_SDK_VERSION,
			&scd,
			swapchainP.GetAddressOf(),
			deviceP.GetAddressOf(),
			NULL,
			deviceContextP.GetAddressOf());

		wrl::ComPtr<ID3D11Texture2D> backBufferP;

		IF_COM_FAIL(swapchainP->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBufferP.GetAddressOf())), "get buffer");

		IF_COM_FAIL(deviceP->CreateRenderTargetView(backBufferP.Get(), nullptr, renderTargetViewP.GetAddressOf()), "create target");
		
		deviceContextP->OMSetRenderTargets(1, renderTargetViewP.GetAddressOf(), NULL);

		D3D11_VIEWPORT viewport;
		ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = width;
		viewport.Height = height;

		//Set the Viewport
		deviceContextP->RSSetViewports(1, &viewport);

		if (!InitShaders())
		{
			EngineException::Log("shader fuckup");
		}
		
		if (!InitScene())
		{
			EngineException::Log("scene fuckup");
		}
		
	}
	catch (COMException exception)
	{
		EngineException::Log(exception);
		return false;
	}

	

	return true;
}

bool Graphics::InitShaders()
{
	
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT, 0, 0,
			D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	UINT numElements = ARRAYSIZE(layout);
	
	
	
	if (!vertexShader.Initialize(this->deviceP, L"..\\x64\\Debug\\VertexShader.cso", layout, numElements))
	{
		EngineException::Log("vertex shader");
		return false;
	}
	
	

	if (!pixelShader.Initialize(this->deviceP, L"..\\x64\\Debug\\PixelShader.cso"))
	{
		EngineException::Log("vertex shader");
		return false;
	}
		
	return true;
}

bool Graphics::InitScene()
{
	Vertex v[] =
	{
		//Vertex(0.0f, 0.0f), //Center
		Vertex(0.0f, -0.6f), //bottom
		Vertex(-0.1f, 0.0f), //Left 
		Vertex(0.1f, 0.0f), //Right 
	};

	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * ARRAYSIZE(v);
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = sizeof(Vertex);

	D3D11_SUBRESOURCE_DATA vertexBufferData;
	ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
	vertexBufferData.pSysMem = v;

	HRESULT hr = deviceP->CreateBuffer(&vertexBufferDesc, &vertexBufferData, vertexBuffer.GetAddressOf());
	if (FAILED(hr))
	{
		EngineException::Log(hr, "Failed to create vertex buffer.");
		return false;
	}

	return true;
}

void Graphics::RenderFrame()
{
	const float color[] = { 0.2f, 0.4f, 0.8f, 1.0f };
	deviceContextP->ClearRenderTargetView(renderTargetViewP.Get(), color);
	
	deviceContextP->IASetInputLayout(vertexShader.GetInputLayout());
	deviceContextP->VSSetShader(vertexShader.GetShader(), NULL, 0);
	deviceContextP->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContextP->PSSetShader(pixelShader.GetShader(), NULL, 0);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	deviceContextP->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);

	deviceContextP->Draw(3, 0);
	
	swapchainP->Present(1u, NULL);
}

void Graphics::ClearBuffer(float red, float green, float blue) noexcept
{
	const float color[] = { red, green, blue, 1.0f };
	deviceContextP->ClearRenderTargetView(renderTargetViewP.Get(), color);
}
/*
ID3D11InputLayout* Graphics::GetInputLayout()
{
	return vertexShader.inputLayoutP.Get();
}
*/

void Graphics::BasicTri()
{
	try
	{
		struct Vert
		{
			float x;
			float y;
		};

		const Vert verts[] =
		{
			{0.0f, 0.5f},
			{0.5f, -0.5f},
			{-0.5f, -0.5f}
		};

		const UINT stride = sizeof(Vert);
		const UINT offset = 0u;

		wrl::ComPtr<ID3D11Buffer> vertexBufferP;
		D3D11_BUFFER_DESC bd = {};
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.CPUAccessFlags = 0u;
		bd.MiscFlags = 0u;
		bd.ByteWidth = sizeof(verts);
		bd.StructureByteStride = sizeof(Vert);

		D3D11_SUBRESOURCE_DATA srd = {};
		srd.pSysMem = verts;

		IF_COM_FAIL(deviceP->CreateBuffer(&bd, &srd, &vertexBufferP), "fail");

		// bind vertex buffer to pipeline
		deviceContextP->IASetVertexBuffers(0u, 1u, &vertexBufferP, &stride, &offset);
		deviceContextP->Draw(3u, 0u);
	}
	catch (COMException exception)
	{
		EngineException::Log(exception);
		
	}
}

/*
void Graphics::errLog(COMException& exception)
{
	std::wstring error_message = exception.what();
	MessageBoxW(NULL, error_message.c_str(), L"Error", MB_ICONERROR);
}
*/
