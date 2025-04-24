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

	cb_light.data.ambientColor = XMFLOAT3(1.0f, 0.8f, 0.8f);
	cb_light.data.ambientStrength = .5f;
	cb_light.data.lightColor = XMFLOAT3(1.0f, 1.0f, 1.0f);
	cb_light.data.lightStrength = 1.0f;
	cb_light.data.lightPosition = XMFLOAT3(8.0f, 12.0f, -4.0f);
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
	DirectX::XMFLOAT3 eye(0.0f, 0.0f, 0.0f);
	camera.SetPosition(2.0f, 8.0f, -16.0f);
	//camera.SetLookAtPos(eye);
	camera.SetProjectionValues(fovDeg, aspectRatio, 0.1f, 1000.0f);
	DirectX::XMMATRIX cameraVP = camera.GetViewMatrix() * camera.GetProjectionMatrix();

	//////////////
	// Set up mesh geometry
	//////////////

	DirectX::XMMATRIX initTransform = DirectX::XMMatrixIdentity();
	plane = std::make_unique<Mesh>(this->device.Get(), this->deviceContext.Get(), initTransform, cb_vert);
	cube = std::make_unique<Mesh>(this->device.Get(), this->deviceContext.Get(), initTransform, cb_vert);
	animatedPlane = std::make_unique<Mesh>(this->device.Get(), this->deviceContext.Get(), initTransform, cb_vert);
	cylinder = std::make_unique<Mesh>(this->device.Get(), this->deviceContext.Get(), initTransform, cb_vert);

	//std::unique_ptr<Mesh> cube2 = std::make_unique<Mesh>(&cube);

	// setup for Plane geometry
	//const int planePointsX = 80;
	//const int planePointsZ = 80;
	//const int planeVertCount = planePointsX * planePointsZ;

	const float xLimit1 = -16.0f;
	const float xLimit2 = 16.0f;
	const float zLimit1 = -16.0f;
	const float zLimit2 = 16.0f;
	const int numPoints = 400;

	const int planeVertCount = numPoints * numPoints;

	// the number of total indices in the triangle array, triangles * 3
	const int planeTriCount = (numPoints - 1) * (numPoints - 1) * 2 * 3;

	const int wavePointsX = 200;
	const int wavePointsZ = 200;
	const int waveVertCount = wavePointsX * wavePointsZ;

	// the number of total indices in the triangle array, triangles * 3
	const int waveTriCount = (wavePointsX - 1) * (wavePointsZ - 1) * 2 * 3;


	//DWORD planeTris[planeTriCount];

	std::unique_ptr<DWORD[]> planeTris = std::make_unique<DWORD[]>(planeTriCount);

	//DWORD waveTris[waveTriCount];
	std::unique_ptr<DWORD[]> waveTris = std::make_unique<DWORD[]>(waveTriCount);

	DWORD cubeTris[36];

	plane->initMesh(planeVertCount, planeTriCount);
	animatedPlane->initMesh(waveVertCount, waveTriCount);
	cube->initMesh(8, 36);

	//if (!plane->buildPlane(planePointsX, planePointsZ))
	//if (!plane->buildPlane(xLimit1, xLimit2, zLimit1, zLimit2, numPoints))
	//{
	//	EngineException::Log("scene fuckup");
	//}
	if (!cube->buildCube(2.0f))
	{
		EngineException::Log("scene fuckup");
	}

	float h = 10.0f;
	float bRad = 3.0f;
	float tRad = 3.0f;
	int hDiv = 200;
	int rDiv = 64;

	int cylinderVertCount = (hDiv + 2) * (rDiv);
	int cylinderTriCount = (hDiv + 1) * (rDiv) * 2 * 3;
	
	
	XMVECTOR tempPos;
	instanceData = std::make_unique<InstancePosition[]>(numMeshes);
	
	for (int i = 0; i < numMeshes; i++)
	{
		float dx = 6.0;
		float dz = 6.0;
		tempPos = XMVectorSet(i * dx, 0.0, i * dz, 0.0);
		XMStoreFloat3(&instData[i].pos, tempPos);
	}
	for (int i = 0; i < numMeshes; i++)
	{
		float dx = 6.0;
		float dz = 6.0;
		instanceData[i].pos = XMFLOAT3(i * dx, 0.0, i * dz);
	}


	cylinder->initMesh(cylinderVertCount, cylinderTriCount);
	//cylinder->buildCylinder(h, bRad, tRad, hDiv, rDiv);
	//cylinder->initBuffers();
	//cylinder->initInstances((instData));

	return true;
}


void Renderer::RenderSetup()
{
	// set up world
	// id matrix implies the origin at 0, 0, 0
	DirectX::XMMATRIX world = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX transform = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX viewProjection = camera.GetViewMatrix() * camera.GetProjectionMatrix();

	const float xLimit1 = -16.0f;
	const float xLimit2 = 16.0f;
	const float zLimit1 = -16.0f;
	const float zLimit2 = 16.0f;
	const int numPoints = 400;
	static float paramSet[3] = { 0.0f, 0.0f, 0.0f };
	static float paramSet2[3] = { 1.0f, 1.0f, 1.0f };
	static float param = 1.0f;
	const int planeVertCount = numPoints * numPoints;

	// the number of total indices in the triangle array, triangles * 3
	//const int planeTriCount = (numPoints - 1) * (numPoints - 1) * 2 * 3;

	//plane->buildPlane(xLimit1, xLimit2, zLimit1, zLimit2, numPoints, paramSet[0], paramSet[1], paramSet[2]);
	//plane->draw(camera.GetViewMatrix() * camera.GetProjectionMatrix());

	//animatedPlane->buildWave(200, 200, 0.1);
	//animatedPlane->draw(camera.GetViewMatrix() * camera.GetProjectionMatrix());
	

	float h = 10.0f;
	float bRad = 2.0f;
	float tRad = 1.5f;
	//int hDiv = 128;
	//int rDiv = 256;
	int hDiv = 200;
	int rDiv = 64;

	
	cylinder->buildCylinder(h, bRad, tRad, hDiv, rDiv, paramSet[0], paramSet[1], paramSet[2], paramSet2[0]);
	cylinder->initBuffers();
	//cylinder->scaleMesh(0.0f, 0.001f, 0.0f);
	//cylinder->setTransformMatrix(cylinder->getScaleMatrix());
	//cylinder->draw(viewProjection);
	//cylinder->drawFast(viewProjection);


	for (int i = 0; i < numMeshes; i++)
	{
		cylinder->initPosition(instanceData[i].pos.x, instanceData[i].pos.y, instanceData[i].pos.z);
		cylinder->initScale(paramSet2[0], paramSet2[1], paramSet2[2]);
		cylinder->setTransformMatrix(cylinder->getScaleMatrix());

		cylinder->drawFast(viewProjection);
	}

	// Start the Dear ImGui frame
	static int counter = 0;
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	//Create ImGui Test Window
	ImGui::Begin("Test");

	//ImGui::DragFloat3("Parameters", paramSet, 0.05f, 0.0f, 2.0f);
	//ImGui::DragFloat3("Parameters2", paramSet2, 0.05f, 0.5f, 2.0f);
	ImGui::SliderFloat3("Parameters", paramSet, 0.0f, 2.0f);
	ImGui::SliderFloat3("Parameters2", paramSet2, 0.5f, 2.0f);
	ImGui::SliderFloat("Param", &param, 0.001, 20.00);
	ImGui::Text(("verts: " + std::to_string(cylinder->vertCount)).c_str());
	/*
	for (int i = 0; i < cylinder->vertCount; i++)
	{
		ImGui::Text(std::to_string(cylinder->vertices[i].pos.x).c_str());
		ImGui::Text(std::to_string(cylinder->vertices[i].pos.y).c_str());
		ImGui::Text(std::to_string(cylinder->vertices[i].pos.z).c_str());
		ImGui::Text("      ");
	}
	*/
	ImGui::Text(("tris: " + std::to_string(cylinder->triCount)).c_str());
	/*
	for (int i = 0; i < cylinder->triCount; i+=3)
	{
		ImGui::Text( (std::to_string(cylinder->tris[i]) + " " + std::to_string(cylinder->tris[i+1]) + " " + std::to_string(cylinder->tris[i+2])).c_str());
	}
	*/

	ImGui::End();

	ImGui::Begin("Test2");
	if (ImGui::Button("Test"))
		counter += 1;
	std::string clicks = "Instance Position 3: " + std::to_string(instData[numMeshes - 1].pos.x);
	ImGui::Text(clicks.c_str());
	std::string info = "Instances: " + std::to_string(numMeshes);
	ImGui::Text(info.c_str());
	ImGui::End();
	//Assemble Together Draw Data
	ImGui::Render();
	//Render Draw Data
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}



