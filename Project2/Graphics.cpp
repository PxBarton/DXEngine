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


		//Describe our Depth/Stencil Buffer
		D3D11_TEXTURE2D_DESC depthStencilDesc;
		depthStencilDesc.Width = width;
		depthStencilDesc.Height = height;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;

		HRESULT hr = deviceP->CreateTexture2D(&depthStencilDesc, NULL, this->depthStencilBuffer.GetAddressOf());
		if (FAILED(hr)) //If error occurred
		{
			EngineException::Log(hr, "Failed to create depth stencil buffer.");
			return false;
		}

		hr = deviceP->CreateDepthStencilView(this->depthStencilBuffer.Get(), NULL, this->depthStencilView.GetAddressOf());
		if (FAILED(hr)) //If error occurred
		{
			EngineException::Log(hr, "Failed to create depth stencil view.");
			return false;
		}
		
		deviceContextP->OMSetRenderTargets(1, renderTargetViewP.GetAddressOf(), depthStencilView.Get());

		//Create depth stencil state
		D3D11_DEPTH_STENCIL_DESC depthstencildesc;
		ZeroMemory(&depthstencildesc, sizeof(D3D11_DEPTH_STENCIL_DESC));

		depthstencildesc.DepthEnable = true;
		depthstencildesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ALL;
		depthstencildesc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;

		hr = deviceP->CreateDepthStencilState(&depthstencildesc, this->depthStencilState.GetAddressOf());
		if (FAILED(hr))
		{
			EngineException::Log(hr, "Failed to create depth stencil state.");
			return false;
		}

		D3D11_VIEWPORT viewport;
		ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = width;
		viewport.Height = height;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		//Set the Viewport
		deviceContextP->RSSetViewports(1, &viewport);



		if (!InitShaders())
		{
			EngineException::Log("shader fuckup");
		}
		
		if (!buildShape())
		{
			EngineException::Log("scene fuckup");
		}

		//Create Rasterizer State
		D3D11_RASTERIZER_DESC rasterizerDesc;
		ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));

		rasterizerDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
		rasterizerDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_BACK;
		rasterizerDesc.AntialiasedLineEnable = true;
		hr = deviceP->CreateRasterizerState(&rasterizerDesc, this->rasterizerState.GetAddressOf());
		if (FAILED(hr))
		{
			EngineException::Log(hr, "rasterizer state.");
			return false;
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
		{"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
			D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, 
			D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  }
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


bool Graphics::InitScene(Vertex v[], DWORD i[], UINT lenV, UINT lenI)
//bool Graphics::InitScene(std::vector<Vertex> v, std::vector<DWORD> i, UINT lenV, UINT lenI)
{



	// ARRAYSIZE(v)
	HRESULT hr = vertexBuffer.Initialize(deviceP.Get(), &v[0], lenV);
	if (FAILED(hr))
	{
		EngineException::Log(hr, "vertex buffer");
		return false;
	}

	// ARRAYSIZE(indices)
	hr = indicesBuffer.Initialize(deviceP.Get(), &i[0], lenI);
	if (FAILED(hr))
	{
		EngineException::Log(hr, "index buffer");
		return false;
	}
	
	hr = constBuffer.Initialize(deviceP.Get(), deviceContextP.Get());
	if (FAILED(hr))
	{
		EngineException::Log(hr, "constant buffer");
		return false;
	}

	return true;
}


void Graphics::RenderFrame()
{


	const float color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	deviceContextP->ClearRenderTargetView(renderTargetViewP.Get(), color);
	deviceContextP->ClearDepthStencilView(this->depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	deviceContextP->IASetInputLayout(vertexShader.GetInputLayout());
	deviceContextP->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContextP->RSSetState(rasterizerState.Get());
	deviceContextP->OMSetDepthStencilState(this->depthStencilState.Get(), 0);
	
	deviceContextP->VSSetShader(vertexShader.GetShader(), NULL, 0);
	deviceContextP->PSSetShader(pixelShader.GetShader(), NULL, 0);

	//UINT stride = sizeof(Vertex);
	UINT offset = 0;

	
	//constBuffer.data.mat = DirectX::XMMatrixScaling(0.5f, 0.5f, 1.0f);
	DirectX::XMMATRIX world = DirectX::XMMatrixIdentity();
	static DirectX::XMVECTOR eye = DirectX::XMVectorSet(2.0f, 1.5f, -3.0f, 0.0f);
	static DirectX::XMVECTOR lookAt = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	static DirectX::XMVECTOR upDir = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(eye, lookAt, upDir);
	float fovDeg = 60.0f;
	float fovRad = (fovDeg / 360.0f) * DirectX::XM_2PI;
	float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
	float nearZ = 0.1f;
	float farZ = 1000.0f;
	DirectX::XMMATRIX projectionMat = DirectX::XMMatrixPerspectiveFovLH(fovRad, aspectRatio, nearZ, farZ);

	constBuffer.data.mat = world * view * projectionMat;
	constBuffer.data.mat = DirectX::XMMatrixTranspose(constBuffer.data.mat);

	constBuffer.data.xOffset = 0.0f;
	constBuffer.data.yOffset = 0.0f;
	if (!constBuffer.ApplyChanges())
	{
		return;
	}
	deviceContextP->VSSetConstantBuffers(0, 1, constBuffer.GetAddressOf());
	deviceContextP->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), vertexBuffer.StridePtr(), &offset);
	deviceContextP->IASetIndexBuffer(indicesBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	deviceContextP->DrawIndexed(indicesBuffer.BufferSize(), 0, 0);
	
	swapchainP->Present(1u, NULL);
}


bool Graphics::buildShape()
{
	Vertex vertList[] =
	{
		//Vertex(0.0f, 0.0f), //Center
		Vertex(0.0f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f), //bottom
		Vertex(-0.5f, -0.3f, 0.0f, 1.0f, 0.0f, 1.0f), //bottom
		Vertex(-0.2f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f), //Left 
		Vertex(0.2f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f), //Right 
		Vertex(0.5f, -0.3f, 0.0f, 1.0f, 1.0f, 0.0f), //bottom
	};

	// DWORD alias for unsigned long
	DWORD indexList[] =
	{
		0, 1, 2,
		0, 2, 3,
		0, 3, 4
	};

	// 3 axis
	Vertex vertList2[] =
	{
		Vertex(-10.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f),
		Vertex(10.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f),
		Vertex(0.0f, -10.0f, 0.0f, 0.0f, 1.0f, 0.0f),
		Vertex(0.0f, 10.0f, 0.0f, 0.0f, 1.0f, 0.0f),
		Vertex(0.0f, 0.0f, -10.0f, 0.0f, 0.0f, 1.0f),
		Vertex(0.0f, 0.0f, 10.0f, 0.0f, 0.0f, 1.0f)
	};

	// x axis
	DWORD indexList2[] =
	{
		0, 1, 2, 3, 4, 5

	};

	std::vector<Vertex> vertList3;
	vertList3.push_back(Vertex(-10.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f));
	vertList3.push_back(Vertex(10.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f));

	std::vector<DWORD> indexList3;
	indexList3.push_back(0);
	indexList3.push_back(1);

	// cube
	Vertex vertList4[] =
	{
		Vertex(-1.f, -1.f, 1.f, 1.f, 1.f, 1.f),
		Vertex(-1.f, -1.f, -1.f, 1.f, 1.f, 1.f),
		Vertex(1.f, -1.f, -1.f, 1.f, 1.f, 1.f),
		Vertex(1.f, -1.f, 1.f, 1.f, 1.f, 1.f),
		Vertex(-1.f, 1.f, 1.f, 1.f, 1.f, 1.f),
		Vertex(-1.f, 1.f, -1.f, 1.f, 1.f, 1.f),
		Vertex(1.f, 1.f, -1.f, 1.f, 1.f, 1.f),
		Vertex(1.f, 1.f, 1.f, 1.f, 1.f, 1.f)
	};

	DWORD triList4[] =
	{
		// sides
		0, 3, 4,
		4, 3, 7,

		3, 2, 7,
		7, 2, 6,

		2, 1, 6,
		6, 1, 5,

		1, 0, 5,
		5, 0, 4,

		// bottom
		0, 1, 3,
		3, 1, 2,

		// top
		4, 7, 5,
		5, 7, 6


	};

	DirectX::XMFLOAT3 triNorm1 = triNormal(vertList4[0], vertList4[3], vertList4[4]);

	calcNormalsV(vertList4, triList4);

	InitScene(vertList4, triList4, ARRAYSIZE(vertList4), ARRAYSIZE(triList4));
	//InitScene(vertList3, indexList3, vertList3.size(), indexList3.size());
	return true;
}


DirectX::XMFLOAT3 Graphics::triNormal(Vertex& A, Vertex& B, Vertex& C)
{
	// convert triangle Vertex.pos to vectors
	DirectX::XMVECTOR vA = DirectX::XMLoadFloat3(&A.pos);
	DirectX::XMVECTOR vB = DirectX::XMLoadFloat3(&B.pos);
	DirectX::XMVECTOR vC = DirectX::XMLoadFloat3(&C.pos);

	// define vectors for cross
	DirectX::XMVECTOR s = DirectX::XMVectorSubtract(vB, vA);
	DirectX::XMVECTOR t = DirectX::XMVectorSubtract(vC, vA);

	//DirectX::XMFLOAT3 u(&B.pos.x - &A.pos.x, &B.pos.y - &A.pos.y, &B.pos.z - &A.pos.z);
	//DirectX::XMFLOAT3 v(&C.pos.x - &A.pos.x, &C.pos.y - &A.pos.y, &C.pos.z - &A.pos.z);
	//DirectX::XMVECTOR U = DirectX::XMLoadFloat3(&u);
	//DirectX::XMVECTOR V = DirectX::XMLoadFloat3(&v);

	DirectX::XMVECTOR norm1 = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(s, t));

	//DirectX::XMVECTOR norm2 = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(U, V));

	DirectX::XMFLOAT3 norm;
	DirectX::XMStoreFloat3(&norm, norm1);

	return (norm);
}


// XMVECTOR version
DirectX::XMVECTOR Graphics::triNormalV(Vertex& A, Vertex& B, Vertex& C)
{
	// convert Vertex.pos to vectors
	DirectX::XMVECTOR vA = DirectX::XMLoadFloat3(&A.pos);
	DirectX::XMVECTOR vB = DirectX::XMLoadFloat3(&B.pos);
	DirectX::XMVECTOR vC = DirectX::XMLoadFloat3(&C.pos);

	// define vectors for cross
	DirectX::XMVECTOR s = DirectX::XMVectorSubtract(vB, vA);
	DirectX::XMVECTOR t = DirectX::XMVectorSubtract(vC, vA);

	//DirectX::XMFLOAT3 u(&B.pos.x - &A.pos.x, &B.pos.y - &A.pos.y, &B.pos.z - &A.pos.z);
	//DirectX::XMFLOAT3 v(&C.pos.x - &A.pos.x, &C.pos.y - &A.pos.y, &C.pos.z - &A.pos.z);
	//DirectX::XMVECTOR U = DirectX::XMLoadFloat3(&u);
	//DirectX::XMVECTOR V = DirectX::XMLoadFloat3(&v);

	DirectX::XMVECTOR normalV = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(s, t));

	//DirectX::XMVECTOR norm2 = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(U, V));


	return (normalV);
}

void Graphics::calcNormals(Vertex verts[], DWORD tris[])
{
	for (int i = 0; i < (sizeof(tris)) / 3; i++)
	{
		int index1 = tris[i * 3 + 0];
		int index2 = tris[i * 3 + 1];
		int index3 = tris[i * 3 + 2];

		DirectX::XMFLOAT3 triNorm = triNormal(verts[index1], verts[index2], verts[index3]);

		verts[index1].normal.x += triNorm.x;
		verts[index1].normal.y += triNorm.y;
		verts[index1].normal.z += triNorm.z;

		verts[index2].normal.x += triNorm.x;
		verts[index2].normal.y += triNorm.y;
		verts[index2].normal.z += triNorm.z;

		verts[index3].normal.x += triNorm.x;
		verts[index3].normal.y += triNorm.y;
		verts[index3].normal.z += triNorm.z;
	}


}

void Graphics::calcNormalsV(Vertex verts[], DWORD tris[])
{
	for (int i = 0; i < (sizeof(tris)) / 3; i++)
	{
		int index1 = tris[i * 3 + 0];
		int index2 = tris[i * 3 + 1];
		int index3 = tris[i * 3 + 2];

		DirectX::XMVECTOR normalV = triNormalV(verts[index1], verts[index2], verts[index3]);

		verts[index1].normalV = DirectX::XMVectorAdd(verts[index1].normalV, normalV);
		verts[index2].normalV = DirectX::XMVectorAdd(verts[index2].normalV, normalV);
		verts[index3].normalV = DirectX::XMVectorAdd(verts[index3].normalV, normalV);
		
	}

	for (int i = 0; i < sizeof(verts); i++)
	{
		verts[i].normalV = DirectX::XMVector3Normalize(verts[i].normalV);
	}

	/*
	DirectX::XMFLOAT3 Graphics::vertNormal(Vertex verts[], DWORD tris[])
	{
		DirectX::XMFLOAT3 norm(0.f, 0.f, 0.f);
		return norm;
	}
	*/

}
