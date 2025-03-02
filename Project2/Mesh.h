#pragma once
#include "Vertex.h"
#include "Buffers.h"

class Mesh
{
public:
	Mesh(ID3D11Device* device,
		ID3D11DeviceContext* deviceContext,
		
		//Microsoft::WRL::ComPtr<ID3D11Device> deviceP,
		//Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContextP,
		const DirectX::XMMATRIX& transformMatrix,
		ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader);

	Mesh() {};

	std::unique_ptr<Vertex[]> vertices = nullptr;
	int vertexCounter = 0;
	int triCounter = 0;
	//Mesh(const Mesh& mesh);
	void Draw(const DirectX::XMMATRIX& worldMatrix, const DirectX::XMMATRIX& viewProjectionMatrix);
	const DirectX::XMMATRIX& GetTransformMatrix();

	bool buildCube(float size, DWORD tris[]);
	bool buildPlane(int xCount, int zCount, Vertex vertices[], DWORD tris[], float xAxis[], float zAxis[]);
	bool buildPlane(int xCount, int zCount, DWORD tris[]);
	std::vector<XMFLOAT3> triNormals{};

	//DirectX::XMFLOAT3 triNormal(Vertex& A, Vertex& B, Vertex& C);
	// XMVECTOR version
	DirectX::XMVECTOR triNormalV(Vertex& A, Vertex& B, Vertex& C);

	bool calcNormals(Vertex verts[], DWORD tris[], int size, int numV);
	// overload
	bool calcNormals(DWORD tris[], int size, int numV);
	// XMVECTOR version
	void calcNormalsV(Vertex verts[], DWORD tris[]);

	void initMeshBuilder(int totalVerts, int totalMeshes)
	{
		vertices = std::make_unique<Vertex[]>(totalVerts);
		//vertexBuffers = std::make_unique<VertexBuffer<Vertex>[]>(totalMeshes);
		//indexBuffers = std::make_unique<IndexBuffer[]>(totalMeshes);
	}

private:
	VertexBuffer<Vertex> vertexBuffer;
	//std::unique_ptr<VertexBuffer<Vertex>[]> vertexBuffers = nullptr;
	IndexBuffer indexBuffer;
	//std::unique_ptr<IndexBuffer[]> indexBuffers = nullptr;
	//Microsoft::WRL::ComPtr<ID3D11Device> device;
	//Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;
	ID3D11Device* device = nullptr;
	ID3D11DeviceContext* deviceContext = nullptr;
	DirectX::XMMATRIX transformMatrix;
	ConstantBuffer<CB_VS_vertexshader>* cb_vs_vertexshader = nullptr;
};