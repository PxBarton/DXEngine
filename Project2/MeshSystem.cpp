#include "MeshSystem.h"


MeshSystem::MeshSystem(
	ID3D11Device* device,
	ID3D11DeviceContext* deviceContext,
	ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader,
	Camera& camera)
{
	this->device = device;
	this->deviceContext = deviceContext;
	this->cb_vs_vertexshader = &cb_vs_vertexshader;
	this->camera = &camera;
}

MeshSystem::MeshSystem()
{}

void MeshSystem::initSystem(int totalCount, Camera& camera, ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader)
{
	this->cb_vs_vertexshader = &cb_vs_vertexshader;
	this->camera = &camera;
	meshArray = std::make_unique<std::unique_ptr<Mesh>[]>(totalCount);
}

void MeshSystem::ringSystem(float radius, int meshCount)
{
	
}

void MeshSystem::gridSystem(float xLength, float zLength, int xCount, int zCount, float param)
{
	viewProjMat = camera->GetViewMatrix() * camera->GetProjectionMatrix();
	int totalCount = xCount * zCount;
	std::unique_ptr<float[]> xAxis = std::make_unique<float[]>(xCount);
	std::unique_ptr<float[]> zAxis = std::make_unique<float[]>(zCount);
	xLength += param;
	zLength += param;
	float dx = xLength / xCount;
	float dz = zLength / zCount;
	DirectX::XMMATRIX initTransform = DirectX::XMMatrixIdentity();

	for (int i = 0; i < xCount; i++)
	{
		xAxis[i] = (-xLength / 2) + i * dx;
	}

	for (int i = 0; i < zCount; i++)
	{
		zAxis[i] = (-zLength / 2) + i * dz;
	}

	for (int i = 0; i < xCount; i++)
	{
		for (int j = 0; j < zCount; j++)
		{
			meshArray[i * zCount + j]->initPosition(xAxis[i], 0.0, zAxis[j]);
			meshArray[i * zCount + j]->draw(viewProjMat);
			//meshVector[i * zCount + j].initPosition(xAxis[i], 0.0, zAxis[j]);
			//meshVector[i * zCount + j].draw(viewProjMat);
		}
	}

}
