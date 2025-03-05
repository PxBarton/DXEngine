#pragma once
#include "Vertex.h"
#include "Buffers.h"

class Mesh
{
public:
	Mesh(ID3D11Device* device,
		ID3D11DeviceContext* deviceContext,
		const XMMATRIX& transformMatrix,
		ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader);

	Mesh() {};
	Timer timer;

	std::unique_ptr<Vertex[]> vertices = nullptr;
	std::unique_ptr<DWORD[]> tris = nullptr;
	int vertCount = 0;
	int triCount = 0;
	int t = 0;
	
	void translate(float x, float y, float z);
	void rotate(float x, float y, float z);
	void Draw(const XMMATRIX& viewProjectionMatrix);
	const XMMATRIX& GetTransformMatrix();

	bool buildCube(float size);
	bool buildPlane(int xCount, int zCount);
	bool wave(int xCount, int zCount, float step);
	std::vector<XMFLOAT3> triNormals{};

	//DirectX::XMFLOAT3 triNormal(Vertex& A, Vertex& B, Vertex& C);
	// XMVECTOR version
	XMVECTOR triNormalV(Vertex& A, Vertex& B, Vertex& C);

	bool calcNormals(Vertex verts[], DWORD tris[], int size, int numV);
	// overload
	bool calcNormals();
	// XMVECTOR version
	void calcNormalsV(Vertex verts[], DWORD tris[]);

	void initMesh(int vertCount, int triCount)
	{
		vertices = std::make_unique<Vertex[]>(vertCount);
		tris = std::make_unique<DWORD[]>(triCount);
		this->vertCount = vertCount;
		this->triCount = triCount;
	}

	void initRotation(float x, float y, float z);
	void setRotation(float x, float y, float z);
	void initPosition(float x, float y, float z);
	void setPosition(float x, float y, float z);

	void setWorldMatrix();

private:
	VertexBuffer<Vertex> vertexBuffer;
	IndexBuffer indexBuffer;
	ID3D11Device* device = nullptr;
	ID3D11DeviceContext* deviceContext = nullptr;
	ConstantBuffer<CB_VS_vertexshader>* cb_vs_vertexshader = nullptr;

	XMVECTOR positionVec = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR rotationVec = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMFLOAT3 position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMMATRIX worldMatrix = XMMatrixIdentity();
	XMMATRIX transformMatrix = XMMatrixIdentity();
};