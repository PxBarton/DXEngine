#include "Mesh.h"


Mesh::Mesh(
	ID3D11Device* device,
	ID3D11DeviceContext* deviceContext,
	
	//Microsoft::WRL::ComPtr<ID3D11Device> device,
	//Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext,
	const DirectX::XMMATRIX& transformMatrix,
	ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader)
{
	this->device = device;
	this->deviceContext = deviceContext;
	this->transformMatrix = transformMatrix;
	this->cb_vs_vertexshader = &cb_vs_vertexshader;
	//this->textures = textures;
}

//Mesh::Mesh() {}

/*
Mesh::Mesh(const Mesh& mesh)
{
	this->deviceContext = mesh.deviceContext;
	this->indexBuffer = mesh.indexBuffer;
	this->vertexBuffer = mesh.vertexBuffer;
	//this->textures = mesh.textures;
	this->transformMatrix = mesh.transformMatrix;
}
*/
void Mesh::Draw(const DirectX::XMMATRIX& worldMatrix, const DirectX::XMMATRIX& viewProjectionMatrix)
{
	
	
	this->deviceContext->VSSetConstantBuffers(0, 1, this->cb_vs_vertexshader->GetAddressOf());

	this->cb_vs_vertexshader->data.wvpMatrix = transformMatrix * worldMatrix * viewProjectionMatrix; //Calculate World-View-Projection Matrix
	this->cb_vs_vertexshader->data.worldMatrix = transformMatrix * worldMatrix; //Calculate World
	this->cb_vs_vertexshader->ApplyChanges();

	UINT offset = 0;

	this->deviceContext->IASetVertexBuffers(0, 1, this->vertexBuffer.GetAddressOf(), this->vertexBuffer.StridePtr(), &offset);
	this->deviceContext->IASetIndexBuffer(this->indexBuffer.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
	this->deviceContext->DrawIndexed(this->indexBuffer.BufferSize(), 0, 0);
}

const DirectX::XMMATRIX& Mesh::GetTransformMatrix()
{
	return this->transformMatrix;
}

bool Mesh::buildCube(float size, DWORD tris[])
{
	// cube
	Vertex vertList[] =
	{
		Vertex(-1.f, -1.f, 1.f),
		Vertex(-1.f, -1.f, -1.f),
		Vertex(1.f, -1.f, -1.f),
		Vertex(1.f, -1.f, 1.f),
		Vertex(-1.f, 1.f, 1.f),
		Vertex(-1.f, 1.f, -1.f),
		Vertex(1.f, 1.f, -1.f),
		Vertex(1.f, 1.f, 1.f)
	};

	DWORD triList[] =
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
	
	for (int i = 0; i <= 35; i++)
	{
		tris[i] = triList[i];
	}

	this->vertexCounter += 8;
	this->triCounter += 36;
	
	return true;
}

bool Mesh::buildPlane(int xCount, int zCount, Vertex vertices[], DWORD tris[], float xAxis[], float zAxis[])
{
	float xLim = 12.0;
	float yLim = 12.0;
	float step = .1;

	//const int xCount = 20;
	//const int zCount = 20;
	const int vCount = xCount * zCount;
	const int triCount = (xCount - 1) * (zCount - 1) * 2 * 3;

	for (int i = 0; i < xCount; i++)
	{
		xAxis[i] = i * step;
	}
	for (int i = 0; i < zCount; i++)
	{
		zAxis[i] = i * step;
	}
	double pi = 3.1415926535;
	int vInd = 0;
	for (int i = 0; i < xCount; i++)
	{
		for (int j = 0; j < zCount; j++)
		{
			vertices[vInd].assign(xAxis[i], .25 * cos(xAxis[i] * pi / 2) * .25 * sin(zAxis[j] * pi / 2) + .25 * cos(xAxis[i] * zAxis[j]), zAxis[j] * pi/2);
			//vertices[vInd].assign(xAxis[i], 0.0f, zAxis[j]);
			vInd++;
		}
	}


	int tInd = 0;
	for (int i = 0; i < xCount - 1; i++)
	{
		for (int j = 0; j < zCount - 1; j++)
		{
			tris[tInd] = (i * zCount) + j;
			tInd++;

			tris[tInd] = (i * zCount) + j + 1;
			tInd++;

			tris[tInd] = (i * zCount) + j + zCount;
			tInd++;

			tris[tInd] = (i * zCount) + j + 1;
			tInd++;

			tris[tInd] = (i * zCount) + j + zCount + 1;
			tInd++;

			tris[tInd] = (i * zCount) + j + zCount;
			tInd++;
		}
	}

	int count = sizeof(tris);

	calcNormals(vertices, tris, triCount, vCount);

	

	return true;
}

//  overloaded to use MeshBuilder vertices 

bool Mesh::buildPlane(int xCount, int zCount, DWORD tris[])
{
	//float xAxis[xCount]; must be allocated on the heap using new or uniqueptr
	//float zAxis[zCount];

	std::unique_ptr<float[]> xAxis = std::make_unique<float[]>(xCount);
	std::unique_ptr<float[]> zAxis = std::make_unique<float[]>(xCount);
	float xLim = 12.0;
	float yLim = 12.0;
	float step = .1;

	int vCount = xCount * zCount;
	const int triCount = (xCount - 1) * (zCount - 1) * 2 * 3;

	for (int i = 0; i < xCount; i++)
	{
		xAxis[i] = i * step;
	}
	for (int i = 0; i < zCount; i++)
	{
		zAxis[i] = i * step;
	}
	double pi = 3.1415926535;
	int vInd = 0 + this->vertexCounter;
	for (int i = 0; i < xCount; i++)
	{
		for (int j = 0; j < zCount; j++)
		{
			this->vertices[vInd].assign(xAxis[i], .25 * cos(xAxis[i] * pi / 2) * .25 * sin(zAxis[j] * pi / 2) + .25 * cos(xAxis[i] * zAxis[j]), zAxis[j] * pi / 2);
			//vertices[vInd].assign(xAxis[i], 0.0f, zAxis[j]);
			vInd++;
		}
	}

	int tInd = 0 + this->triCounter;
	for (int i = 0; i < xCount - 1; i++)
	{
		for (int j = 0; j < zCount - 1; j++)
		{
			tris[tInd] = (i * zCount) + j;
			tInd++;

			tris[tInd] = (i * zCount) + j + 1;
			tInd++;

			tris[tInd] = (i * zCount) + j + zCount;
			tInd++;

			tris[tInd] = (i * zCount) + j + 1;
			tInd++;

			tris[tInd] = (i * zCount) + j + zCount + 1;
			tInd++;

			tris[tInd] = (i * zCount) + j + zCount;
			tInd++;
		}
	}

	//calcNormals(tris, triCount, vCount);
	/*
	HRESULT hr = vertexBuffer.Initialize(this->device, &vertices[0], vCount);
	if (FAILED(hr))
	{
		EngineException::Log(hr, "vertex buffer");

	}
	// ARRAYSIZE(i)
	hr = indexBuffer.Initialize(this->device, tris, triCount);
	if (FAILED(hr))
	{
		EngineException::Log(hr, "index buffer");

	}
	*/
	return true;
}


// XMVECTOR version
// returns a normal vector for a triangle, given 3 vertices
DirectX::XMVECTOR Mesh::triNormalV(Vertex& A, Vertex& B, Vertex& C)
{
	// convert Vertex.pos to vectors
	DirectX::XMVECTOR vA = DirectX::XMLoadFloat3(&A.pos);
	DirectX::XMVECTOR vB = DirectX::XMLoadFloat3(&B.pos);
	DirectX::XMVECTOR vC = DirectX::XMLoadFloat3(&C.pos);

	// define vectors for cross
	DirectX::XMVECTOR s = DirectX::XMVectorSubtract(vB, vA);
	DirectX::XMVECTOR t = DirectX::XMVectorSubtract(vC, vA);

	DirectX::XMVECTOR normalV = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(s, t));


	return (normalV);
}

bool Mesh::calcNormals(DWORD tris[], int size, int numV)
{
	for (int i = 0; i < size / 3; i++)
	{
		int index1 = tris[i * 3 + 0];
		int index2 = tris[i * 3 + 1];
		int index3 = tris[i * 3 + 2];

		DirectX::XMVECTOR triNormV = triNormalV(this->vertices[index1], this->vertices[index2], this->vertices[index3]);
		XMFLOAT3 normal;
		DirectX::XMStoreFloat3(&normal, triNormV);
		//triNormals.push_back(normal);

		this->vertices[index1].normalV = DirectX::XMVectorAdd(this->vertices[index1].normalV, triNormV);
		this->vertices[index2].normalV = DirectX::XMVectorAdd(this->vertices[index2].normalV, triNormV);
		this->vertices[index3].normalV = DirectX::XMVectorAdd(this->vertices[index3].normalV, triNormV);


	}

	for (int i = 0; i < numV; i++)
	{
		this->vertices[i].normalV = DirectX::XMVector3Normalize(this->vertices[i].normalV);
		DirectX::XMStoreFloat3(&this->vertices[i].normal, this->vertices[i].normalV);
	}

	return true;
}


bool Mesh::calcNormals(Vertex verts[], DWORD tris[], int size, int numV)
{
	for (int i = 0; i < size / 3; i++)
	{
		int index1 = tris[i * 3 + 0];
		int index2 = tris[i * 3 + 1];
		int index3 = tris[i * 3 + 2];

		DirectX::XMVECTOR triNormV = triNormalV(verts[index1], verts[index2], verts[index3]);
		XMFLOAT3 normal;
		DirectX::XMStoreFloat3(&normal, triNormV);
		triNormals.push_back(normal);

		verts[index1].normalV = DirectX::XMVectorAdd(verts[index1].normalV, triNormV);
		verts[index2].normalV = DirectX::XMVectorAdd(verts[index2].normalV, triNormV);
		verts[index3].normalV = DirectX::XMVectorAdd(verts[index3].normalV, triNormV);

		//DirectX::XMVECTOR norm1;
		//DirectX::XMVECTOR norm2;
		//DirectX::XMVECTOR norm3;

	}

	for (int i = 0; i < numV; i++)
	{
		verts[i].normalV = DirectX::XMVector3Normalize(verts[i].normalV);
		DirectX::XMStoreFloat3(&verts[i].normal, verts[i].normalV);
	}
	/*
	if (verts[(sizeof(verts)) / 2].normal.y < 0.1)
	{
		return false;
	}
	*/
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////


// do not use

void Mesh::calcNormalsV(Vertex verts[], DWORD tris[])
{
	for (int i = 0; i < (sizeof(tris)) / 3; i++)
	{
		int index1 = tris[i * 3 + 0];
		int index2 = tris[i * 3 + 1];
		int index3 = tris[i * 3 + 2];

		DirectX::XMVECTOR triNormV = triNormalV(verts[index1], verts[index2], verts[index3]);

		verts[index1].normalV = DirectX::XMVectorAdd(verts[index1].normalV, triNormV);
		verts[index2].normalV = DirectX::XMVectorAdd(verts[index2].normalV, triNormV);
		verts[index3].normalV = DirectX::XMVectorAdd(verts[index3].normalV, triNormV);

	}

	for (int i = 0; i < sizeof(verts); i++)
	{
		verts[i].normalV = DirectX::XMVector3Normalize(verts[i].normalV);
	}


}


/*
DirectX::XMFLOAT3 Mesh::triNormal(Vertex& A, Vertex& B, Vertex& C)
{
	// convert triangle Vertex.pos to vectors
	DirectX::XMVECTOR vA = DirectX::XMLoadFloat3(&A.pos);
	DirectX::XMVECTOR vB = DirectX::XMLoadFloat3(&B.pos);
	DirectX::XMVECTOR vC = DirectX::XMLoadFloat3(&C.pos);

	// define vectors for cross
	DirectX::XMVECTOR s = DirectX::XMVectorSubtract(vB, vA);
	DirectX::XMVECTOR t = DirectX::XMVectorSubtract(vC, vA);

	DirectX::XMVECTOR norm1 = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(s, t));

	//DirectX::XMVECTOR norm2 = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(U, V));

	DirectX::XMFLOAT3 norm;
	DirectX::XMStoreFloat3(&norm, norm1);

	return (norm);
*/



///////////////////////////////////////////////////

//	python plane

/*

origin = [0, 0, 0]
xLim = 40
yLim = 40
step = .5
xAxis = np.arange(-xLim/2, xLim/2+1, step)
yAxis = np.arange(-yLim/2, yLim/2+1, step)
x = len(xAxis)
y = len(yAxis)

def makePlane(X, Y) :
	points = []
	#X = np.arange(C[0], C[0] + Xaxis, 1)
	#Y = np.arange(C[1], C[1] + Yaxis, 1)
	#X = np.linspace(0, Xaxis, 10)
	#Y = np.linspace(0, Yaxis, 10)
	for i in range(len(X)) :
		for j in range(len(Y)) :
			coord = [X[i], Y[j], 0]
			points.append(coord)
			return points

for i in range(x-1):
	for j in range(y-1):
		face = [(i*y)+j, (i*y)+j+1, (i*y)+j+y+1, (i*y)+j+y]
		tri1 = [(i*y)+j, (i*y)+j+1, (i*y)+j+y]
		tri2 = [(i*y)+j+1, (i*y)+j+y+1, (i*y)+j+y]
		faces.append(face)

*/

/*
void ModelRenderer::UpdateInstanceBuffer()
{
	NS::InstanceStateFactory<int> * stateFactory = nullptr;
	stateFactory = (NS::InstanceStateFactory<int> *) NS::ModelFacade::GetInstanceStateFactory();

	// std::vector<NS::Vertex> instances;

	unsigned int memoryAllocationSizeInBytes = sizeof(Position) * 10;

	std::vector<int8_t> buffer(memoryAllocationSizeInBytes);

	int instanceCount = 0;
	int stride = sizeof(Position);

	for(auto & kv : stateFactory->Instances)
	{
		// Push vector arrays into Memory Buffer
		memcpy(&buffer[0] + instanceCount * stride, & kv.second->Position, stride);

		instanceCount ++;
	}

	D3D11_MAPPED_SUBRESOURCE resource;

	HRESULT hResult = DeviceManager::DeviceContext->
		Map(ModelRenderer::InstanceBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);

	// resource.pData = instances.data();
	memcpy(resource.pData, buffer.data(), sizeof(Position) * instanceCount);

	DeviceManager::DeviceContext->Unmap(ModelRenderer::InstanceBuffer.Get(), 0);

	// ModelRenderer::BindInstanceBuffer();

	// delete instances.data();
}
*/