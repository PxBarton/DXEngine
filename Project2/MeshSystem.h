#pragma once

#include <vector> 
#include "Mesh.h"
#include "Camera.h"

class MeshSystem
{
public:
	
	MeshSystem(ID3D11Device* device,
		ID3D11DeviceContext* deviceContext,
		ConstantBuffer<CB_vertex>& cb_vertex,
		Camera& camera);

	MeshSystem();

	Camera* camera;
	DirectX::XMMATRIX viewProjMat = DirectX::XMMatrixIdentity();
	std::unique_ptr<std::unique_ptr<Mesh>[]> meshArray = nullptr;
	std::unique_ptr<Mesh[]> meshMatrix = nullptr;
	std::vector<Mesh> meshVector = {};

	void initSystem(int totalCount, Camera& camera, ConstantBuffer<CB_vertex>& cb_vertex);
	void ringSystem(float radius, int meshCount);
	void gridSystem(float xLength, float zLength, int xCount, int zCount, float param);
	void stack(float height, float stackCount);

private:
	ID3D11Device* device = nullptr;
	ID3D11DeviceContext* deviceContext = nullptr;
	ConstantBuffer<CB_vertex>* cb_vertex;
};