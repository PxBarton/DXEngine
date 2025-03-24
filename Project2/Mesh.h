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

	Mesh(const Mesh& rvMesh);

	Timer timer;

	std::unique_ptr<Vertex[]> vertices = nullptr;
	std::unique_ptr<DWORD[]> tris = nullptr;
	std::vector<XMFLOAT3> triNormals{};
	int vertCount = 0;
	int triCount = 0;
	int t = 0;
	
	//DirectX::XMFLOAT3 triNormal(Vertex& A, Vertex& B, Vertex& C);
	XMVECTOR triNormalV(Vertex& A, Vertex& B, Vertex& C);

	bool calcNormals(Vertex verts[], DWORD tris[], int size, int numV);
	bool calcNormals();
	void calcNormalsV(Vertex verts[], DWORD tris[]);

	void initRotation(float x, float y, float z);
	void setRotation(float x, float y, float z);
	void initPosition(float x, float y, float z);
	void setPosition(float x, float y, float z);

	bool buildCube(float size);
	bool buildPlane(float xLim1, float xLim2, float zLim1, float zLim2, int numPoints, float param1, float param2, float param3);
	bool buildPlane(int xCount, int zCount);
	bool buildCylinder(float height, float baseRadius, float topRadius, int hDivs, int rDivs);
	bool buildCylinder(float height, float baseRadius, float topRadius, int hDivs, int rDivs, float param1, float param2, float param3, float param4);
	bool buildWave(int xCount, int zCount, float step);

	const XMMATRIX& getTransformMatrix();
	void setWorldMatrix();
	void setTransformMatrix();
	void translate(float x, float y, float z);
	void rotate(float x, float y, float z);

	void initMesh(int vertCount, int triCount);
	void initInstances(std::vector<InstancePosition> instData);
	void initInstances(std::unique_ptr<InstancePosition[]> instanceData);
	void initBuffers();
	void draw(const XMMATRIX& viewProjectionMatrix);
	void drawFast(const XMMATRIX& viewProjectionMatrix, const XMMATRIX& transformMatrix);
	void drawInstances(const XMMATRIX& viewProjectionMatrix);
	

	int getNumInstances()
	{
		return instanceBuffer.BufferSize();
	}

private:
	VertexBuffer<Vertex> vertexBuffer;
	IndexBuffer indexBuffer;
	InstanceBuffer<InstancePosition> instanceBuffer;
	ID3D11Device* device = nullptr;
	ID3D11DeviceContext* deviceContext = nullptr;
	ConstantBuffer<CB_VS_vertexshader>* cb_vs_vertexshader = nullptr;
	ID3D11Buffer* vertInstBuffers[2] = {};

	XMVECTOR positionVec = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR rotationVec = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMFLOAT3 position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMMATRIX worldMatrix = XMMatrixIdentity();
	XMMATRIX transformMatrix = XMMatrixIdentity();
};