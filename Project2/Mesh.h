#pragma once
#include "Vertex.h"
#include "Buffers.h"

class Mesh
{
public:
	Mesh(ID3D11Device* device,
		ID3D11DeviceContext* deviceContext,
		const XMMATRIX& transformMatrix,
		ConstantBuffer<CB_vertex>& cb_vertex);

	Mesh() {};

	Mesh(const Mesh& rvMesh);

	Timer timer;

	std::unique_ptr<Vertex[]> vertices = nullptr;
	std::unique_ptr<DWORD[]> tris = nullptr;
	std::vector<XMFLOAT3> triNormals{};
	int vertCount = 0;
	int triCount = 0;
	int t = 0;
	
	XMVECTOR triNormalV(Vertex& A, Vertex& B, Vertex& C);
	bool calcNormals();
	bool flatNormals();


	bool calcNormals(Vertex verts[], DWORD tris[], int size, int numV);
	
	
	void initRotation(float x, float y, float z);
	void animateRotation(float x, float y, float z);
	void initPosition(float x, float y, float z);
	void animatePosition(float x, float y, float z);
	void initScale(float x, float y, float z);
	void animateScale(float x, float y, float z);
	const XMMATRIX getScaleMatrix();

	bool buildCube(float size);
	bool buildPlane(float xLim1, float xLim2, float zLim1, float zLim2, int numPoints, float param1, float param2, float param3);
	bool buildPlane(int xCount, int zCount);
	bool buildCylinder(float height, float baseRadius, float topRadius, int hDivs, int rDivs);
	bool buildCylinder(float height, float baseRadius, float topRadius, int hDivs, int rDivs, float param1, float param2, float param3, float param4);
	bool buildWave(int xCount, int zCount, float step);
	bool buildPolyStack(int stacks, XMFLOAT3 center, float xSpan, float zSpan, float innerXSpan, float innerZSpan,
		float height1, float height2, float height3, float height4, float out, float twist);

	bool buildCubeFlat(float size);
	bool buildPyramidFlat(float side);

	const XMMATRIX& getTransformMatrix();
	void setWorldMatrix();
	void setTransformMatrix();
	void setTransformMatrix(XMMATRIX newTransform);

	void initMesh(int vertCount, int triCount);
	void initInstances(std::vector<InstancePosition> instData);
	void initInstances(std::unique_ptr<InstancePosition[]> instanceData);
	void initBuffers();
	void draw(const XMMATRIX& viewProjectionMatrix);
	void drawFast(const XMMATRIX& viewProjectionMatrix);
	void drawInstances(const XMMATRIX& viewProjectionMatrix);
	

	int getNumInstances()
	{
		return instanceBuffer.BufferSize();
	}

	
	void drawXZGrid()
	{
		XMFLOAT3 center = XMFLOAT3(0.0, 0.0, 0.0);
		float length = 100;
		int lineCount = 10;
		float lineSpacing = length / lineCount;
		float lineWidth = 0.025;
		
		float xStart = center.x - length / 2;
		float xEnd = center.x + length / 2;
		float zStart = center.z - length / 2;
		float zEnd = center.z + length / 2;

		initMesh(lineCount * 4 * 2, lineCount * 6 * 2);
		
		HRESULT hr = vertexBuffer.Initialize(device, vertices.get(), vertCount);
		if (FAILED(hr))
		{
			EngineException::Log(hr, "vertex buffer");

		}
	}

private:
	VertexBuffer<Vertex> vertexBuffer;
	IndexBuffer indexBuffer;
	InstanceBuffer<InstancePosition> instanceBuffer;
	ID3D11Device* device = nullptr;
	ID3D11DeviceContext* deviceContext = nullptr;
	ConstantBuffer<CB_vertex>* cb_vertex = nullptr;
	ID3D11Buffer* vertInstBuffers[2] = {};

	XMVECTOR positionVec = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR rotationVec = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR scaleVec = XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f);

	XMFLOAT3 position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 scale = XMFLOAT3(1.0f, 1.0f, 1.0f);

	XMMATRIX scaleMatrix = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	XMMATRIX worldMatrix = XMMatrixIdentity();
	XMMATRIX transformMatrix = XMMatrixIdentity();
};