#pragma once

#include <vector> 
#pragma once

#include <DirectXMath.h>
#include <cmath>
#include <vector>
#include <memory>

class MeshSystem
{
public:
	MeshSystem();
	MeshSystem(DirectX::XMFLOAT3 center, int count);
	
	float getCenter();
	float setCenter();
	int getCount();
	void setCount(int meshCount);
	void ringSystem(float radius, int ringCount, int perRingCount);
	void gridSystem(float xLength, float zLength, int xCount, int zCount, float param);
	void stack(float height, float stackCount);
	std::vector<DirectX::XMFLOAT3> getSystemVector();

private:
	DirectX::XMFLOAT3 systemCenter = DirectX::XMFLOAT3(0.0, 0.0, 0.0);
	int systemCount = 0;
	std::unique_ptr<DirectX::XMFLOAT3[]> meshArray = nullptr;
	std::unique_ptr<std::unique_ptr<DirectX::XMFLOAT3>[]> meshGrid = nullptr;
	std::vector<DirectX::XMFLOAT3> meshVector = {};
	std::vector<std::vector<DirectX::XMFLOAT3>> meshVectorGrid = {};
};