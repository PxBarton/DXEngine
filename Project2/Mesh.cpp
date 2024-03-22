#include "Mesh.h"
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

Mesh::Mesh(ID3D11Device* device, 
		ID3D11DeviceContext* deviceContext, 
		std::vector<Vertex>& vertices, 
		std::vector<DWORD>& indices,
		const DirectX::XMMATRIX& transformMatrix,
		ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader)
{
	this->deviceContext = deviceContext;
	//this->textures = textures;
	this->transformMatrix = transformMatrix;

	HRESULT hr = this->vertexBuffer.Initialize(device, vertices.data(), vertices.size());
	//COM_ERROR_IF_FAILED(hr, "Failed to initialize vertex buffer for mesh.");

	hr = this->indexBuffer.Initialize(device, indices.data(), indices.size());
	//COM_ERROR_IF_FAILED(hr, "Failed to initialize index buffer for mesh.");
}
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