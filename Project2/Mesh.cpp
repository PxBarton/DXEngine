#include "Mesh.h"


Mesh::Mesh(
	ID3D11Device* device,
	ID3D11DeviceContext* deviceContext,
	const DirectX::XMMATRIX& transformMatrix,
	ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader)
{
	this->device = device;
	this->deviceContext = deviceContext;
	this->transformMatrix = transformMatrix;
	this->cb_vs_vertexshader = &cb_vs_vertexshader;
	//this->textures = textures;
}

Mesh::Mesh(const Mesh& rvMesh)
{
	this->vertCount = rvMesh.vertCount;
	this->triCount = rvMesh.triCount;
	this->vertices = std::make_unique<Vertex[]>(this->vertCount);
	for (int i = 0; i < vertCount; i++)
	{
		this->vertices[i] = rvMesh.vertices[i];
	}
	this->tris = std::make_unique<DWORD[]>(this->triCount);
	for (int i = 0; i < triCount; i++)
	{
		this->tris[i] = rvMesh.tris[i];
	}
	this->device = rvMesh.device;
	this->deviceContext = rvMesh.deviceContext;
	//vertexBuffer = rvMesh.vertexBuffer;
	//indexBuffer = rvMesh.indexBuffer;
	this->cb_vs_vertexshader = rvMesh.cb_vs_vertexshader;
	
	this->positionVec = rvMesh.positionVec;
	this->rotationVec = rvMesh.rotationVec;
	this->position = rvMesh.position;
	this->rotation = rvMesh.rotation;
	this->worldMatrix = rvMesh.worldMatrix;
	this->transformMatrix = rvMesh.transformMatrix;

	HRESULT hr = this->vertexBuffer.Initialize(device, vertices.get(), vertCount);
	if (FAILED(hr))
	{
		EngineException::Log(hr, "vertex buffer");

	}
	// ARRAYSIZE(i)
	hr = this->indexBuffer.Initialize(device, tris.get(), triCount);
	if (FAILED(hr))
	{
		EngineException::Log(hr, "index buffer");

	}

}

void Mesh::initPosition(float x, float y, float z)
{
	position = XMFLOAT3(x, y, z);
	positionVec = XMLoadFloat3(&position);
	setWorldMatrix();
}

void Mesh::setPosition(float x, float y, float z)
{
	position.x += x;
	position.y += y;
	position.z += z;
	positionVec = XMLoadFloat3(&position);
	setWorldMatrix();
}


void Mesh::initRotation(float x, float y, float z)
{
	rotation = XMFLOAT3(x, y, z);
	rotationVec = XMLoadFloat3(&rotation);
	setWorldMatrix();
}

void Mesh::setRotation(float x, float y, float z)
{
	rotation.x += x;
	rotation.y += y;
	rotation.z += z;
	rotationVec = XMLoadFloat3(&rotation);
	setWorldMatrix();
}

void Mesh::translate(float x, float y, float z)
{
	float time = timer.GetMilisecondsElapsed();
	timer.Restart();
	setPosition(x * time, y * time, z * time);
}

void Mesh::rotate(float x, float y, float z)
{
	float time = timer.GetMilisecondsElapsed();
	timer.Restart();
	setRotation(x * time, y * time, z * time);
}

void Mesh::draw(const DirectX::XMMATRIX& viewProjectionMatrix)
{
	deviceContext->VSSetConstantBuffers(0, 1, cb_vs_vertexshader->GetAddressOf());

	cb_vs_vertexshader->data.wvpMatrix = transformMatrix * worldMatrix * viewProjectionMatrix; //Calculate World-View-Projection Matrix
	cb_vs_vertexshader->data.worldMatrix = transformMatrix * worldMatrix; //Calculate World
	cb_vs_vertexshader->data.wvpMatrix = DirectX::XMMatrixTranspose(cb_vs_vertexshader->data.wvpMatrix);
	cb_vs_vertexshader->ApplyChanges();

	UINT offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), vertexBuffer.StridePtr(), &offset);
	deviceContext->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
	deviceContext->DrawIndexed(indexBuffer.BufferSize(), 0, 0);
}

void Mesh::initInstances(std::vector<InstancePosition> instData)
{
	HRESULT hr = instanceBuffer.Initialize(device, &instData[0], 8);
	if (FAILED(hr))
	{
		EngineException::Log(hr, "instance buffer");

	}
	vertInstBuffers[0] = vertexBuffer.Get();
	vertInstBuffers[1] = instanceBuffer.Get();
}

void Mesh::initInstances(std::unique_ptr<InstancePosition[]> instanceData)
{
	HRESULT hr = instanceBuffer.Initialize(device, &instanceData[0], 8);
	if (FAILED(hr))
	{
		EngineException::Log(hr, "instance buffer");

	}
	vertInstBuffers[0] = vertexBuffer.Get();
	vertInstBuffers[1] = instanceBuffer.Get();
}

void Mesh::drawInstances(const XMMATRIX& viewProjectionMatrix)
{
	
	
	deviceContext->VSSetConstantBuffers(0, 1, cb_vs_vertexshader->GetAddressOf());

	cb_vs_vertexshader->data.wvpMatrix = transformMatrix * worldMatrix * viewProjectionMatrix; //Calculate World-View-Projection Matrix
	cb_vs_vertexshader->data.worldMatrix = transformMatrix * worldMatrix; //Calculate World
	cb_vs_vertexshader->data.wvpMatrix = DirectX::XMMatrixTranspose(cb_vs_vertexshader->data.wvpMatrix);
	cb_vs_vertexshader->ApplyChanges();

	UINT offset = 0;
	const UINT* vStride = vertexBuffer.StridePtr();
	const UINT* iStride = instanceBuffer.StridePtr();
	std::vector<const UINT*> strides{ vStride, iStride };
	deviceContext->IASetVertexBuffers(0, 2, &vertInstBuffers[0], strides[0], &offset);
	deviceContext->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
	deviceContext->DrawIndexedInstanced(indexBuffer.BufferSize(), 8, 0, 0, 0);
}

void Mesh::setWorldMatrix()
{
	// change to quaternions
	worldMatrix = XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z) * XMMatrixTranslation(position.x, position.y, position.z);
}

const DirectX::XMMATRIX& Mesh::getTransformMatrix()
{
	return this->transformMatrix;
}

bool Mesh::buildCube(float size)
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
	
	for (int i = 0; i <= vertCount - 1; i++)
	{
		vertices[i] = vertList[i];
	}
	for (int i = 0; i <= triCount - 1; i++)
	{
		tris[i] = triList[i];
	}

	calcNormals();

	HRESULT hr = vertexBuffer.Initialize(device, vertices.get(), 8);
	if (FAILED(hr))
	{
		EngineException::Log(hr, "vertex buffer");

	}
	// ARRAYSIZE(i)
	hr = indexBuffer.Initialize(device, tris.get(), 36);
	if (FAILED(hr))
	{
		EngineException::Log(hr, "index buffer");

	}
	
	return true;
}



//  overloaded to use MeshBuilder vertices 

bool Mesh::buildPlane(int xCount, int zCount)
{

	std::unique_ptr<float[]> xAxis = std::make_unique<float[]>(xCount);
	std::unique_ptr<float[]> zAxis = std::make_unique<float[]>(zCount);
	float xLim = 12.0;
	float yLim = 12.0;
	float step = .1;


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
			//this->vertices[vInd].assign(xAxis[i], .25 * cos(xAxis[i] / 4 * pi / 2) * .25 * sin(zAxis[j] * pi / 2) + .25 * cos(xAxis[i] * zAxis[j]), zAxis[j] * pi / 2);
			vertices[vInd].assign(xAxis[i], 0.0f, zAxis[j]);
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

	calcNormals();
	
	HRESULT hr = vertexBuffer.Initialize(device, vertices.get(), vertCount);
	if (FAILED(hr))
	{
		EngineException::Log(hr, "vertex buffer");

	}
	// ARRAYSIZE(i)
	hr = indexBuffer.Initialize(device, tris.get(), triCount);
	if (FAILED(hr))
	{
		EngineException::Log(hr, "index buffer");

	}
	
	return true;
}


bool Mesh::buildPlane(float xLim1, float xLim2, float zLim1, float zLim2, int numPoints, float param1, float param2, float param3)
{

	std::unique_ptr<float[]> xAxis = std::make_unique<float[]>(numPoints);
	std::unique_ptr<float[]> zAxis = std::make_unique<float[]>(numPoints);
	
	float xStep = (xLim2 - xLim1) / numPoints;
	float zStep = (zLim2 - zLim1) / numPoints;


	for (int i = 0; i < numPoints; i++)
	{
		xAxis[i] = xLim1 + i * xStep;
	}
	for (int i = 0; i < numPoints; i++)
	{
		zAxis[i] = zLim1 + i * zStep;
	}
	double pi = 3.1415926535;
	int vInd = 0;
	for (int i = 0; i < numPoints; i++)
	{
		for (int j = 0; j < numPoints; j++)
		{
			//this->vertices[vInd].assign(xAxis[i], .25 * cos(xAxis[i] / 4 * pi / 2) * .25 * sin(zAxis[j] * pi / 2) + .25 * cos(xAxis[i] * zAxis[j]), zAxis[j] * pi / 2);

			//vertices[vInd].assign(xAxis[i], param1 * 2 * cos(xAxis[i] * pi / 8) * sin(zAxis[j] * pi / 8 * param2), zAxis[j]);

			//vertices[vInd].assign(xAxis[i], 2 * (cos(2 * xAxis[i] * pi / 2) + cos( 2 * zAxis[j] * pi / 2 )) * (exp(-abs(.3 * xAxis[i])) * exp(-abs(.3 * zAxis[j]))), zAxis[j]);

			//vertices[vInd].assign(xAxis[i], exp(-abs(xAxis[i])) * exp(-abs(zAxis[j])), zAxis[j]);

			vertices[vInd].assign(xAxis[i], 2 * param1 * (cos(2 * param2 * xAxis[i] * pi / 2) + cos(2 * zAxis[j] * pi / 2)) * (exp(-abs(.3 * param3 * xAxis[i])) * exp(-abs(.3 * zAxis[j]))), zAxis[j]);

			//vertices[vInd].assign(xAxis[i], 0.0f, zAxis[j]);

			vInd++;
		}
	}

	int tInd = 0;
	for (int i = 0; i < numPoints - 1; i++)
	{
		for (int j = 0; j < numPoints - 1; j++)
		{
			tris[tInd] = (i * numPoints) + j;
			tInd++;

			tris[tInd] = (i * numPoints) + j + 1;
			tInd++;

			tris[tInd] = (i * numPoints) + j + numPoints;
			tInd++;

			tris[tInd] = (i * numPoints) + j + 1;
			tInd++;

			tris[tInd] = (i * numPoints) + j + numPoints + 1;
			tInd++;

			tris[tInd] = (i * numPoints) + j + numPoints;
			tInd++;
		}
	}

	calcNormals();

	HRESULT hr = vertexBuffer.Initialize(device, vertices.get(), vertCount);
	if (FAILED(hr))
	{
		EngineException::Log(hr, "vertex buffer");

	}
	// ARRAYSIZE(i)
	hr = indexBuffer.Initialize(device, tris.get(), triCount);
	if (FAILED(hr))
	{
		EngineException::Log(hr, "index buffer");

	}

	return true;
}


bool Mesh::buildWave(int xCount, int zCount, float step)
{
	float time = timer.GetMilisecondsElapsed();
	timer.Restart();
	t += time;

	std::unique_ptr<float[]> xAxis = std::make_unique<float[]>(xCount);
	std::unique_ptr<float[]> zAxis = std::make_unique<float[]>(xCount);
	float xLim = 12.0;
	float yLim = 12.0;

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
			this->vertices[vInd].assign(xAxis[i], .5 * cos( xCount / 4 + (.0005 * t)) * (.15 * sin(zAxis[j] * pi / 2 + (.001 * t)))  + (.3 * sin(xAxis[i] * 2.5 * zAxis[j] + (.001 * t)) ), zAxis[j]);
			// * .25 * sin(zAxis[j] * pi / 2 
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

	calcNormals();

	HRESULT hr = vertexBuffer.Initialize(device, vertices.get(), vertCount);
	if (FAILED(hr))
	{
		EngineException::Log(hr, "vertex buffer");

	}
	// ARRAYSIZE(i)
	hr = indexBuffer.Initialize(device, tris.get(), triCount);
	if (FAILED(hr))
	{
		EngineException::Log(hr, "index buffer");

	}

	return true;
}


bool Mesh::buildCylinder(float height, float baseRadius, float topRadius, int hDivs, int rDivs)
{
	float hSlice = height / (hDivs + 1);    // 0 divs: divide by 1, not zero
	float dRadius = (topRadius - baseRadius) / (hDivs + 1);    // negative if radius grows smaller toward top

	// hDiv = 1 indicates a cylinder divided in half, so 1 + 2 for the base and the top rings = 3 rings
	int ringCount = hDivs + 2;
	int vInd = 0;
	for (int i = 0; i < ringCount; i++)
	{
		float y = i * hSlice;
		float r = baseRadius + i * dRadius;
		float dTheta = 2.0 * XM_PI / rDivs;

		for (int j = 1; j <= rDivs; j++)
		{
			float c = cos(j * dTheta);
			float s = sin(j * dTheta);

			this->vertices[vInd].assign(r * c, y, r * s);
			vInd++;
		}
	}

	int tInd = 0;
	for (int i = 0; i < (hDivs + 1); i++)
	{

		for (int j = 0; j < (rDivs - 1); j++)
		{
			tris[tInd] = i * rDivs + (j + 1);  // 1
			tInd++;

			tris[tInd] = i * rDivs + j;  // 0
			tInd++;

			tris[tInd] = (i + 1) * rDivs + (j + 1);  // 5
			tInd++;

			tris[tInd] = (i + 1) * rDivs + (j + 1);  // 5
			tInd++;

			tris[tInd] = i * rDivs + j;  // 0
			tInd++;

			tris[tInd] = (i + 1) * rDivs + j;  // 4
			tInd++;
		}

		tris[tInd] = (i + 1) * rDivs - rDivs;  // 0
		tInd++;

		tris[tInd] = (i + 1) * rDivs - 1;  // 3
		tInd++;

		tris[tInd] = (i + 1) * rDivs;  // 4
		tInd++;

		tris[tInd] = (i + 1) * rDivs;  // 4
		tInd++;

		tris[tInd] = (i + 1) * rDivs - 1;  // 3
		tInd++;

		tris[tInd] = (i + 2) * rDivs - 1;  // 7
		tInd++;
	}

	calcNormals();

	HRESULT hr = vertexBuffer.Initialize(device, vertices.get(), vertCount);
	if (FAILED(hr))
	{
		EngineException::Log(hr, "vertex buffer");

	}
	// ARRAYSIZE(i)
	hr = indexBuffer.Initialize(device, tris.get(), triCount);
	if (FAILED(hr))
	{
		EngineException::Log(hr, "index buffer");

	}

	

	return true;
}


bool Mesh::buildCylinder(float height, float baseRadius, float topRadius, int hDivs, int rDivs, float param1, float param2, float param3, float param4)
{
	float hSlice = height / (hDivs + 1);    // 0 divs: divide by 1, not zero
	float dRadius = (topRadius - baseRadius) / (hDivs + 1);    // negative if radius grows smaller toward top

	// hDiv = 1 indicates a cylinder divided in half, so 1 + 2 for the base and the top rings = 3 rings
	int ringCount = hDivs + 2;
	int vInd = 0;
	for (int i = 0; i < ringCount; i++)
	{
		float y = i * hSlice;
		//float r = baseRadius + i * dRadius + (param1 * cos(param2 * i * XM_PI / 2) + param3 * sin(i * XM_PI / 16));
		float r = baseRadius + i * dRadius + (param1 * cos(param2 * i * XM_PI / 16));
		float dTheta = 2.0 * XM_PI / rDivs;

		for (int j = 1; j <= rDivs; j++)
		{
			float c = cos(j * dTheta);
			float s = sin(j * dTheta);

			r = r + (param3 * sin(j * XM_PI/(8 * param4)));
			this->vertices[vInd].assign(r * c, y, r * s);
			vInd++;
		}
	}

	int tInd = 0;
	for (int i = 0; i < (hDivs + 1); i++)
	{

		for (int j = 0; j < (rDivs - 1); j++)
		{
			tris[tInd] = i * rDivs + (j + 1);  // 1
			tInd++;

			tris[tInd] = i * rDivs + j;  // 0
			tInd++;

			tris[tInd] = (i + 1) * rDivs + (j + 1);  // 5
			tInd++;

			tris[tInd] = (i + 1) * rDivs + (j + 1);  // 5
			tInd++;

			tris[tInd] = i * rDivs + j;  // 0
			tInd++;

			tris[tInd] = (i + 1) * rDivs + j;  // 4
			tInd++;
		}

		tris[tInd] = (i + 1) * rDivs - rDivs;  // 1
		tInd++;

		tris[tInd] = (i + 1) * rDivs - 1;  // 0
		tInd++;

		tris[tInd] = (i + 1) * rDivs;  // 5
		tInd++;

		tris[tInd] = (i + 1) * rDivs;  // 5
		tInd++;

		tris[tInd] = (i + 1) * rDivs - 1;  // 0
		tInd++;

		tris[tInd] = (i + 2) * rDivs - 1;  // 4
		tInd++;
	}

	calcNormals();

	HRESULT hr = vertexBuffer.Initialize(device, vertices.get(), vertCount);
	if (FAILED(hr))
	{
		EngineException::Log(hr, "vertex buffer");

	}
	// ARRAYSIZE(i)
	hr = indexBuffer.Initialize(device, tris.get(), triCount);
	if (FAILED(hr))
	{
		EngineException::Log(hr, "index buffer");

	}

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

bool Mesh::calcNormals()
{
	for (int i = 0; i < triCount / 3; i++)
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

	for (int i = 0; i < vertCount; i++)
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