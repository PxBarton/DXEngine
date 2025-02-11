#pragma once
#include "Vertex.h"
#include "Buffers.h"

class Mesh
{
public:
	Mesh(ID3D11Device* device, 
		ID3D11DeviceContext* deviceContext, 
		std::vector<Vertex>& vertices, 
		std::vector<DWORD>& indices, 
		const DirectX::XMMATRIX& transformMatrix,
		ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader);

	Mesh() {};

	//Mesh(const Mesh& mesh);
	void Draw(const DirectX::XMMATRIX& worldMatrix, const DirectX::XMMATRIX& viewProjectionMatrix);
	const DirectX::XMMATRIX& GetTransformMatrix();
	bool buildPlane(int xCount, int zCount, Vertex vertices[], DWORD tris[], float xAxis[], float zAxis[]);

	std::vector<XMFLOAT3> triNormals{};

	//DirectX::XMFLOAT3 triNormal(Vertex& A, Vertex& B, Vertex& C);
	// XMVECTOR version
	DirectX::XMVECTOR triNormalV(Vertex& A, Vertex& B, Vertex& C);

	bool calcNormals(Vertex verts[], DWORD tris[], int size, int numV);
	// XMVECTOR version
	void calcNormalsV(Vertex verts[], DWORD tris[]);

private:
	VertexBuffer<Vertex> vertexBuffer;
	IndexBuffer indexBuffer;
	ID3D11Device* device = nullptr;
	ID3D11DeviceContext* deviceContext = nullptr;
	DirectX::XMMATRIX transformMatrix;
	ConstantBuffer<CB_VS_vertexshader>* cb_vs_vertexshader = nullptr;
};