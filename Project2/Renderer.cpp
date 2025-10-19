#include "Renderer.h"

namespace wrl = Microsoft::WRL;

Renderer::Renderer() 
{}

bool Renderer::Init(HWND hWnd, int width, int height)
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
			adapters[0].adapter,
			D3D_DRIVER_TYPE_UNKNOWN,
			NULL,
			NULL,
			NULL,
			0,
			D3D11_SDK_VERSION,
			&scd,
			swapchain.GetAddressOf(),
			device.GetAddressOf(),
			NULL,
			deviceContext.GetAddressOf());

		wrl::ComPtr<ID3D11Texture2D> backBuffer;

		HRESULT hr = swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
		if (FAILED(hr)) //If error occurred
		{
			EngineException::Log(hr, "Failed to create back buffer.");
			return false;
		}

		hr = device->CreateRenderTargetView(backBuffer.Get(), nullptr, renderTargetView.GetAddressOf());
		if (FAILED(hr)) //If error occurred
		{
			EngineException::Log(hr, "Failed to create render target view.");
			return false;
		}

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

		hr = device->CreateTexture2D(&depthStencilDesc, NULL, this->depthStencilBuffer.GetAddressOf());
		if (FAILED(hr)) //If error occurred
		{
			EngineException::Log(hr, "Failed to create depth stencil buffer.");
			return false;
		}

		hr = device->CreateDepthStencilView(this->depthStencilBuffer.Get(), NULL, this->depthStencilView.GetAddressOf());
		if (FAILED(hr)) //If error occurred
		{
			EngineException::Log(hr, "Failed to create depth stencil view.");
			return false;
		}
		
		deviceContext->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView.Get());

		//Create depth stencil state
		D3D11_DEPTH_STENCIL_DESC depthstencildesc;
		ZeroMemory(&depthstencildesc, sizeof(D3D11_DEPTH_STENCIL_DESC));

		depthstencildesc.DepthEnable = true;
		depthstencildesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ALL;
		depthstencildesc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;

		hr = device->CreateDepthStencilState(&depthstencildesc, this->depthStencilState.GetAddressOf());
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
		deviceContext->RSSetViewports(1, &viewport);
		
		//Create Rasterizer State
		D3D11_RASTERIZER_DESC rasterizerDesc;
		ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));

		rasterizerDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
		rasterizerDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
		rasterizerDesc.MultisampleEnable = true;
		rasterizerDesc.AntialiasedLineEnable = true;
		hr = device->CreateRasterizerState(&rasterizerDesc, this->rasterizerState.GetAddressOf());
		if (FAILED(hr))
		{
			EngineException::Log(hr, "rasterizer state.");
			return false;
		}

		//Create sampler description for sampler state
		D3D11_SAMPLER_DESC sampDesc;
		ZeroMemory(&sampDesc, sizeof(sampDesc));
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
		hr = this->device->CreateSamplerState(&sampDesc, this->samplerState.GetAddressOf()); //Create sampler state
		if (FAILED(hr))
		{
			EngineException::Log(hr, "Failed to create sampler state.");
			return false;
		}

		//Create Blend State
		D3D11_RENDER_TARGET_BLEND_DESC rtbd = { 0 };
		rtbd.BlendEnable = true;
		rtbd.SrcBlend = D3D11_BLEND::D3D11_BLEND_SRC_ALPHA;
		rtbd.DestBlend = D3D11_BLEND::D3D11_BLEND_INV_SRC_ALPHA;
		rtbd.BlendOp = D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;
		rtbd.SrcBlendAlpha = D3D11_BLEND::D3D11_BLEND_ONE;
		rtbd.DestBlendAlpha = D3D11_BLEND::D3D11_BLEND_ZERO;
		rtbd.BlendOpAlpha = D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;
		rtbd.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE::D3D11_COLOR_WRITE_ENABLE_ALL;

		D3D11_BLEND_DESC blendDesc = { 0 };
		blendDesc.RenderTarget[0] = rtbd;

		hr = this->device->CreateBlendState(&blendDesc, this->blendState.GetAddressOf());
		if (FAILED(hr))
		{
			EngineException::Log(hr, "Failed to create blend state.");
			return false;
		}

		if (!InitShaders())
		{
			EngineException::Log("shader fuckup");
		}

		// initialize constant buffers when vertex & pixel shaders filled in the mesh object
		SceneSetup();
		
	}
	catch (COMException exception)
	{
		EngineException::Log(exception);
		return false;
	}

	//Setup ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(this->device.Get(), this->deviceContext.Get());
	ImGui::StyleColorsDark();

	return true;
}

///////////////////////////////
// Frame presented to swap chain here
///////////////////////////////
void Renderer::RenderFrame()
{
	const float color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	deviceContext->ClearRenderTargetView(renderTargetView.Get(), color);
	deviceContext->ClearDepthStencilView(this->depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	deviceContext->IASetInputLayout(vertexShader.GetInputLayout());
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->RSSetState(rasterizerState.Get());
	deviceContext->OMSetDepthStencilState(this->depthStencilState.Get(), 0);

	deviceContext->VSSetShader(vertexShader.GetShader(), NULL, 0);
	deviceContext->PSSetShader(pixelShader.GetShader(), NULL, 0);

	UINT offset = 0;

	cb_light.data.ambientColor = XMFLOAT3(0.8f, 0.8f, 1.0f);
	cb_light.data.ambientStrength = .5f;

	cb_light.data.lightColor = XMFLOAT3(1.0f, 1.0f, 1.0f);
	cb_light.data.lightStrength = 0.8f;
	cb_light.data.lightPosition = XMFLOAT3(15.0f, 15.0f, -10.0f);

	if (!cb_light.ApplyChanges())
	{
		return;
	}
	deviceContext->PSSetConstantBuffers(0, 1, cb_light.GetAddressOf());

	RenderSetup();

	swapchain->Present(1u, NULL);
}


///////////////////////////////
// Input Layout desc is here
///////////////////////////////
bool Renderer::InitShaders()
{
	
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
			D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, 
			D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
		{"NORMAL", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
		{"INSTANCEDATA", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 1,
			D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_INSTANCE_DATA, 1  }
	};

	UINT numElements = ARRAYSIZE(layout);
	
	
	if (!vertexShader.Initialize(this->device, L"..\\x64\\Debug\\VertexShader.cso", layout, numElements))
	{
		EngineException::Log("vertex shader problem");
		return false;
	}
	
	
	if (!pixelShader.Initialize(this->device, L"..\\x64\\Debug\\PixelShader.cso"))
	{
		EngineException::Log("pixel shader problem");
		return false;
	}
		
	return true;
}


bool Renderer::SceneSetup()
{
	// Constant buffers, camera. mesh geometry setup

	HRESULT hr = cb_vert.Initialize(this->device.Get(), deviceContext.Get());
	if (FAILED(hr))
	{
		EngineException::Log(hr, "constant buffer: vertex");
		return false;
	}

	hr = cb_light.Initialize(this->device.Get(), deviceContext.Get());
	if (FAILED(hr))
	{
		EngineException::Log(hr, "constant buffer: pixel/light");
		return false;
	}

	hr = DirectX::CreateWICTextureFromFile(device.Get(), L"Resource Files\\tex7.png", nullptr, texture.GetAddressOf());

	/////////////
	// Camera
	/////////////
	float fovDeg = 60.0f;
	float fovRad = (fovDeg / 360.0f) * DirectX::XM_2PI;
	float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
	DirectX::XMFLOAT3 eye(0.0f, 4.0f, 0.0f);
	camera.SetPosition(4.0f, 8.0f, -24.0f);
	camera.SetLookAtPos(eye);
	camera.SetProjectionValues(fovDeg, aspectRatio, 0.1f, 1000.0f);
	DirectX::XMMATRIX cameraVP = camera.GetViewMatrix() * camera.GetProjectionMatrix();

	//////////////
	// Set up mesh geometry
	//////////////

	DirectX::XMMATRIX initTransform = DirectX::XMMatrixIdentity();

	cube = std::make_unique<Mesh>(this->device.Get(), this->deviceContext.Get(), initTransform, cb_vert);
	flatPlane = std::make_unique<Mesh>(this->device.Get(), this->deviceContext.Get(), initTransform, cb_vert);
	plane = std::make_unique<Mesh>(this->device.Get(), this->deviceContext.Get(), initTransform, cb_vert);
	cylinder = std::make_unique<Mesh>(this->device.Get(), this->deviceContext.Get(), initTransform, cb_vert);
	building = std::make_unique<Mesh>(this->device.Get(), this->deviceContext.Get(), initTransform, cb_vert);

	// QuadSystem test
	qSquare = std::make_unique<QuadSystem>();
	qSquare->buildFlatSquare(8.0);
	//qSquare->CCsubdivide(3.0, 2.0, 1.0);
	//qSquare->CCsubdivide(3.0, 2.0, 1.0);

	square = qSquare->convertQuadsToMesh();
	square->initGPU(this->device.Get(), this->deviceContext.Get(), initTransform, cb_vert);
	square->initDraw();
	
	XMFLOAT3 p1 = XMFLOAT3(-2.0, -2.0, -2.0);
	XMFLOAT3 p2 = XMFLOAT3(2.0, -2.0, -2.0);
	XMFLOAT3 p3 = XMFLOAT3(2.0, -2.0, 2.0);
	XMFLOAT3 p4 = XMFLOAT3(-2.0, -2.0, 2.0);

	XMFLOAT3 boxNormal = XMFLOAT3(0.0, 1.0, 0.0);
	XMFLOAT3 boxNormal2 = XMFLOAT3(0.0, 1.0, 0.3);

	std::vector<XMFLOAT3> newPoints = { p1, p2, p3, p4 };
	
	std::array<int, 4> newIndices = { 0, 1, 2, 3 };

	XMFLOAT3 defaultScale = XMFLOAT3(1.0, 1.0, 1.0);
	XMFLOAT3 boxScale = XMFLOAT3(0.5, 0.5, 0.5);
	XMFLOAT3 boxScale2 = XMFLOAT3(1.0, 1.0, 1.0);
	XMFLOAT3 boxScale3 = XMFLOAT3(6.0, 6.0, 6.0);
	XMFLOAT3 boxScale4 = XMFLOAT3(1.0, 1.0, 1.0);
	XMFLOAT3 boxScale5 = XMFLOAT3(0.4, 0.4, 0.4);

	qBox = std::make_unique<QuadSystem>();
	
	/*
	qBox->addPoints(newPoints);
	qBox->buildBox(newIndices, boxNormal, 3.0, defaultScale);
	qBox->buildBox(qBox->getBox(0).farCorners(), boxNormal, 2.0, boxScale);
	qBox->buildBox(qBox->getBox(1).farCorners(), boxNormal, 4.0, boxScale2);
	qBox->buildBox(qBox->getBox(2).farCorners(), boxNormal, 3.0, boxScale3);
	qBox->buildBox(qBox->getBox(3).farCorners(), boxNormal, 1.0, boxScale4);
	qBox->buildBox(qBox->getBox(4).farCorners(), boxNormal, 3.0, boxScale);
	qBox->buildBox(qBox->getBox(5).farCorners(), boxNormal, 2.0, boxScale5);
	qBox->topCap(qBox->getBox(6));
	qBox->bottomCap(qBox->getBox(0));

	

	std::vector<int> sides = { 0, 1, 2, 3 };
	std::vector<float> angles = { -50.0, -50.0, -50.0, -50.0 };
	std::vector<float> angles2 = { -30.0, -30.0, -30.0, -30.0 };


	Branch startingBranch;
	startingBranch.axis = qBox->getBox(6).direction;
	qBox->branch(startingBranch, qBox->bottomCapId, sides, angles2, 0.4);
	for (uint64_t capId : qBox->branchCaps)
	{
		qBox->buildBall(capId, 2.5, true, true);
	}
	qBox->branchCaps.clear();
	qBox->branch(startingBranch, qBox->topCapId, sides, angles, 0.6);
	//qBox->deleteStagedFaces();
	std::vector<uint64_t> currentCaps = qBox->branchCaps; // Get the list of new cap IDs
    //auto& testCap = currentCaps.back();
	qBox->branchCaps.clear();
	//qBox->buildBall(testCap, 1.5, true, true);
	
	//qBox->branch(startingBranch, currentCaps[0], sides, angles, 0.8);
	//qBox->branch(startingBranch, currentCaps[1], sides, angles, 0.8);
	//qBox->branch(startingBranch, currentCaps[2], sides, angles, 0.8);
	//qBox->branch(startingBranch, currentCaps[3], sides, angles, 0.8);
	
	//for (uint64_t& cap : qBox->branchCaps)
	//{
	//	qBox->buildBall(cap, 2.0, true, true);
	//}
	
	for (uint64_t capId : currentCaps)
	{
		//qBox->buildBall(capId, 2.0, true, true);
		//qBox->branch(startingBranch, qBox->capIds.back(), sides, angles, 0.8);
		qBox->branch(startingBranch, capId, sides, angles, 0.8);
		std::vector<uint64_t> moreCaps = qBox->branchCaps;
		qBox->branchCaps.clear();
		for (int i = 0; i < moreCaps.size(); i++)
		{
			qBox->branch(startingBranch, moreCaps[i], sides, angles, 1.5);
			for (uint64_t capId : qBox->branchCaps)
			{
				qBox->buildBall(capId, 1.75, true, true);
			}
			qBox->branchCaps.clear();
		}
		moreCaps.clear();
		//qBox->buildBall(capId, 1.75, true, true);
	}
	
	//qBox->deleteStagedFaces();
	
	//Box test = qBox->getBox(0)
	//std::vector<Face> faceTest = qBox->getFaces();
	
	uint64_t face1Id = qBox->getBox(0).faceIds[0];
	qBox->getFaceById(face1Id).normalV = qBox->calcNormal(face1Id);
	DirectX::XMStoreFloat3(&qBox->getFaceById(face1Id).normal, qBox->getFaceById(face1Id).normalV);
	XMFLOAT3 newNormal1 = qBox->getFaceById(face1Id).normal;
	//qBox->replaceFace(face1Id, newNormal1, 6.0, XMFLOAT3(0.6, 0.6, 0.6), true, false);
	XMFLOAT3 testNormal1 = XMFLOAT3(newNormal1.x, newNormal1.y - 0.5, newNormal1.z);
	qBox->replaceFace(face1Id, testNormal1, 6.0, XMFLOAT3(0.4, 0.4, 0.4), true, false);
	//qBox->buildBall(qBox->capIds.back(), 2.0, true, true);
	//qBox->branch(startingBranch, qBox->capIds.back(), sides, angles, 0.6);

	// how do I get the new endcap id for the box? 

	int face2Id = qBox->getBox(0).faceIds[1];
	qBox->getFaceById(face2Id).normalV = qBox->calcNormal(face2Id);
	DirectX::XMStoreFloat3(&qBox->getFaceById(face2Id).normal, qBox->getFaceById(face2Id).normalV);
	XMFLOAT3 newNormal2 = qBox->getFaceById(face2Id).normal;
	//qBox->replaceFace(face2Id, newNormal2, 6.0, XMFLOAT3(0.6, 0.6, 0.6), true, false);
	XMFLOAT3 testNormal2 = XMFLOAT3(newNormal2.x, newNormal2.y - 0.5, newNormal2.z);
	qBox->replaceFace(face2Id, testNormal2, 6.0, XMFLOAT3(0.4, 0.4, 0.4), true, false);
	//qBox->buildBall(qBox->capIds.back(), 2.0, true, true);

	int face3Id = qBox->getBox(0).faceIds[2];
	qBox->getFaceById(face3Id).normalV = qBox->calcNormal(face3Id);
	DirectX::XMStoreFloat3(&qBox->getFaceById(face3Id).normal, qBox->getFaceById(face3Id).normalV);
	XMFLOAT3 newNormal3 = qBox->getFaceById(face3Id).normal;
	//qBox->replaceFace(face3Id, newNormal3, 6.0, XMFLOAT3(0.6, 0.6, 0.6), true, false);
	XMFLOAT3 testNormal3 = XMFLOAT3(newNormal3.x, newNormal3.y - 0.5, newNormal3.z);
	qBox->replaceFace(face3Id, testNormal3, 6.0, XMFLOAT3(0.4, 0.4, 0.4), true, false);
	//qBox->buildBall(qBox->capIds.back(), 2.0, true, true);

	int face4Id = qBox->getBox(0).faceIds[3];
	qBox->getFaceById(face4Id).normalV = qBox->calcNormal(face4Id);
	DirectX::XMStoreFloat3(&qBox->getFaceById(face4Id).normal, qBox->getFaceById(face4Id).normalV);
	XMFLOAT3 newNormal4 = qBox->getFaceById(face4Id).normal;
	//qBox->replaceFace(face4Id, newNormal4, 6.0, XMFLOAT3(0.6, 0.6, 0.6), true, false);
	XMFLOAT3 testNormal4 = XMFLOAT3(newNormal4.x, newNormal4.y - 0.5, newNormal4.z);
	qBox->replaceFace(face4Id, testNormal4, 6.0, XMFLOAT3(0.4, 0.4, 0.4), true, false);
	//qBox->buildBall(qBox->capIds.back(), 2.0, true, true);

	int face5Id = qBox->getBox(2).faceIds[0];
	qBox->getFaceById(face5Id).normalV = qBox->calcNormal(face5Id);
	DirectX::XMStoreFloat3(&qBox->getFaceById(face5Id).normal, qBox->getFaceById(face5Id).normalV);
	XMFLOAT3 newNormal5 = qBox->getFaceById(face5Id).normal;
	qBox->replaceFace(face5Id, newNormal5, 4.0, XMFLOAT3(0.6, 0.6, 0.6), true, false);

	int face6Id = qBox->getBox(2).faceIds[1];
	qBox->getFaceById(face6Id).normalV = qBox->calcNormal(face6Id);
	DirectX::XMStoreFloat3(&qBox->getFaceById(face6Id).normal, qBox->getFaceById(face6Id).normalV);
	XMFLOAT3 newNormal6 = qBox->getFaceById(face6Id).normal;
	qBox->replaceFace(face6Id, newNormal6, 4.0, XMFLOAT3(0.6, 0.6, 0.6), true, false);

	int face7Id = qBox->getBox(2).faceIds[2];
	qBox->getFaceById(face7Id).normalV = qBox->calcNormal(face7Id);
	DirectX::XMStoreFloat3(&qBox->getFaceById(face7Id).normal, qBox->getFaceById(face7Id).normalV);
	XMFLOAT3 newNormal7 = qBox->getFaceById(face7Id).normal;
	qBox->replaceFace(face7Id, newNormal7, 4.0, XMFLOAT3(0.6, 0.6, 0.6), true, false);

	int face8Id = qBox->getBox(2).faceIds[3];
	qBox->getFaceById(face8Id).normalV = qBox->calcNormal(face8Id);
	DirectX::XMStoreFloat3(&qBox->getFaceById(face8Id).normal, qBox->getFaceById(face8Id).normalV);
	XMFLOAT3 newNormal8 = qBox->getFaceById(face8Id).normal;
	qBox->replaceFace(face8Id, newNormal8, 4.0, XMFLOAT3(0.6, 0.6, 0.6), true, false);
	
	qBox->deleteStagedFaces();
	*/

	qBox->buildCylinder(qBox->basePoint, boxNormal, 8.0, 4.0, .4, 4, 12);
	Cylinder& newCyl = qBox->getCylinder(0);
	for (int i = 0; i < newCyl.slices[1].size(); i += 2)
	{
		qBox->getFaceById(newCyl.slices[1][i]).normalV = qBox->calcNormal(newCyl.slices[1][i]);
		DirectX::XMStoreFloat3(&qBox->getFaceById(newCyl.slices[1][i]).normal, qBox->getFaceById(newCyl.slices[1][i]).normalV);
		XMFLOAT3 n = qBox->getFaceById(newCyl.slices[1][i]).normal;
		qBox->replaceFace(newCyl.slices[1][i], n, 4.0, XMFLOAT3(0.6, 0.6, 0.6), true, false);
	}
	uint64_t cylCapId = qBox->topCylinderCap(newCyl);
	//qBox->triangulateNgon(cylCapId);

	qBox->deleteStagedFaces();

	//qBox->CCsubdivide(2.8, 2.0, 1.2);
	//qBox->CCsubdivide(5.0, 2.0, 4.0);
	//qBox->CCsubdivide(3.0, 2.0, 1.0);
	//qBox->CCsubdivide(3.0, 2.0, 1.0);
	//qBox->CCsubdivide(3.0, 2.0, 1.0);
	//qBox->CCsubdivide(3.0, 2.0, 1.0);
	//qBox->CCsubdivide(3.0, 2.0, 1.0);

	qBox->CCsubdivideNgon(3.0, 2.0, 1.0);
	qBox->CCsubdivideNgon(3.0, 2.0, 1.0);
	//qBox->CCsubdivideNgon(3.0, 2.0, 1.0);

	box = qBox->convertFacesToMesh();
	box->initGPU(this->device.Get(), this->deviceContext.Get(), initTransform, cb_vert);
	box->initDraw();
	


	// cube setup
	cube->initMesh(8, 36);
	cube->buildCube(2.0f);
	//cube->buildCubeFlat(2.0f);

	// flat plane setup
	const float xLimit1 = -16.0f;
	const float xLimit2 = 16.0f;
	const float zLimit1 = -16.0f;
	const float zLimit2 = 16.0f;
	const int numPoints = 8;

	const int planeVertCount = numPoints * numPoints;

	// the number of total indices in the triangle array, triangles * 3
	const int planeTriCount = (numPoints - 1) * (numPoints - 1) * 2 * 3;

	flatPlane->initMesh(planeVertCount, planeTriCount);
	flatPlane->buildPlane(xLimit1, xLimit2, zLimit1, zLimit2, numPoints, 0.0, 0.0, 0.0);

	// parameter plane setup
	plane->initMesh(planeVertCount, planeTriCount);
	
	// cylinder setup
	float h = 10.0f;
	float bRad = 3.0f;
	float tRad = 3.0f;
	int hDiv = 8;
	int rDiv = 6;

	int cylinderVertCount = (hDiv + 2) * (rDiv);
	int cylinderTriCount = (hDiv + 1) * (rDiv) * 2 * 3;
	
	cylinder->initMesh(cylinderVertCount, cylinderTriCount);
	cylinder->buildCylinder(h, bRad, tRad, hDiv, rDiv);
	//cylinder->initBuffers();

	//building->buildPolyStack(4, origin, 6, 4, 4, 3, 1, 1, 1, 1, 1.2, 0);

	return true;
}


void Renderer::RenderSetup()
{
	// set up world
	// id matrix implies the origin at 0, 0, 0
	DirectX::XMMATRIX world = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX transform = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX viewProjection = camera.GetViewMatrix() * camera.GetProjectionMatrix();

	
	static float param1 = 10.0f;
	static float param2 = 10.0f;
	static float param3 = 6.0f;
	static float param4 = 6.0f;
	static float param5 = 1.0f;
	static float param6 = 1.0f;
	static float param7 = 1.0f;
	static float param8 = 1.0f;
	static float param9 = 1.5f;

	static float param10 = 1.0f;
	static float param11 = 1.0f;
	static float param12 = 1.0f;
	static float param13 = 1.0f;

	XMFLOAT3* module = building->buildModule(param1,
		param2, param3, param4,
		param5, param6, param7, param8, param9);

	building->buildPolyStack(8, origin, module,
		param1, param2, param3,
		param4, param5, param6, param7, param8, param9, 0.0, 0.0,
		param10, param11, param12, param13 );

	//building->draw(viewProjection);

	//square->draw(viewProjection);
	box->draw(viewProjection);

	// Start the Dear ImGui frame
	static int counter = 0;
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	//Create ImGui Test Window
	ImGui::Begin("Parameters");

	
	ImGui::SliderFloat("Param1", &param1, 0.001, 20.00);
	ImGui::SliderFloat("Param2", &param2, 0.001, 20.00);
	ImGui::SliderFloat("Param3", &param3, 0.001, 10.00);
	ImGui::SliderFloat("Param4", &param4, 0.001, 10.00);
	ImGui::SliderFloat("Param5", &param5, 0.001, 3.00);
	ImGui::SliderFloat("Param6", &param6, 0.001, 3.00);
	ImGui::SliderFloat("Param7", &param7, 0.001, 3.00);
	ImGui::SliderFloat("Param8", &param8, 0.001, 3.00);
	ImGui::SliderFloat("Param9", &param9, 0.001, 4.00);
	//ImGui::SliderFloat("Param10", &param10, -2.00, 2.00);
	//ImGui::SliderFloat("Param11", &param11, -2.00, 2.00);
	//ImGui::SliderFloat("Param12", &param12, -2.00, 2.00);
	//ImGui::SliderFloat("Param13", &param13, -2.00, 2.00);
	//ImGui::SliderFloat("Param14", &param1, 0.001, 20.00);

	//ImGui::SliderFloat3("Parameters", paramSet3, 0.5f, 12.0f);
	//ImGui::SliderFloat3("Parameters2", paramSet4, 0.25f, 12.0f);
	//ImGui::SliderFloat3("Parameters3", paramSet5, 0.25f, 3.0f);
	ImGui::End();

	ImGui::Begin("Info");

	XMFLOAT3 cameraPos = camera.GetPosition();
	std::string info = "Camera Position: " + std::to_string(cameraPos.x) + "  " + std::to_string(cameraPos.y) + "  " + std::to_string(cameraPos.z);
	ImGui::Text(info.c_str());

	ImGui::End();
	//Assemble Together Draw Data
	ImGui::Render();
	//Render Draw Data
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}


/*

const float xLimit1 = -16.0f;
	const float xLimit2 = 16.0f;
	const float zLimit1 = -16.0f;
	const float zLimit2 = 16.0f;
	const int numPoints = 8;

	static float paramSet[3] = { 0.1f, 0.1f, 0.1f };
	static float paramSet2[3] = { 0.5f, 0.5f, 0.5f };

	static float paramSet3[3] = { 10.0f, 10.0f, 6.0f };
	static float paramSet4[3] = { 6.0f, 1.0f, 1.0f };
	static float paramSet5[3] = { 1.0f, 1.0f, 1.5f };


	plane->buildPlane(xLimit1, xLimit2, zLimit1, zLimit2, numPoints, paramSet[0], paramSet[1], paramSet[2]);

	//plane->draw(viewProjection);

	//flatPlane->draw(viewProjection);

	//cylinder->draw(viewProjection);

	//cube->draw(viewProjection);

	//ImGui::SliderFloat3("Parameters", paramSet, 0.1f, 2.0f);
	//ImGui::SliderFloat3("Parameters2", paramSet2, 0.5f, 2.0f);
*/



