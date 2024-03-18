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

	//Mesh(const Mesh& mesh);
	void Draw(const DirectX::XMMATRIX& worldMatrix, const DirectX::XMMATRIX& viewProjectionMatrix);
	const DirectX::XMMATRIX& GetTransformMatrix();
private:
	VertexBuffer<Vertex> vertexBuffer;
	IndexBuffer indexBuffer;
	ID3D11Device* device = nullptr;
	ID3D11DeviceContext* deviceContext = nullptr;
	DirectX::XMMATRIX transformMatrix;
	ConstantBuffer<CB_VS_vertexshader>* cb_vs_vertexshader = nullptr;
};