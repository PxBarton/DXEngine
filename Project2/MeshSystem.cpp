#include "MeshSystem.h"

MeshSystem::MeshSystem()
{

}

MeshSystem::MeshSystem(DirectX::XMFLOAT3 center, int count)
{
	systemCenter = center;
	systemCount = count;
}
int MeshSystem::getCount()
{
	return systemCount;
}


std::vector<DirectX::XMFLOAT3> MeshSystem::getSystemVector()
{
	return meshVector;
}

void MeshSystem::ringSystem(float radius, int ringCount, int perRingCount)
{
	
}

void MeshSystem::gridSystem(float xLength, float zLength, int xCount, int zCount, float param = 0.0)
{
	systemCount = xCount * zCount;
	float dx = xLength / xCount;
	float dz = zLength / zCount;
	float xStart = systemCenter.x - xLength / 2;
	float xEnd = systemCenter.x + xLength / 2;
	float zStart = systemCenter.z - zLength / 2;
	float zEnd = systemCenter.z + zLength / 2;
	
	for (int i = 0; i < systemCount; i++)
	{
		meshVector.push_back(DirectX::XMFLOAT3(0.0, 0.0, 0.0));
	}

	for (int i = 0; i < xCount; i++)
	{
		for (int j = 0; j < zCount; j++)
		{
			meshVector[(i * zCount) + j].x = xStart + i * dx;
			meshVector[(i * zCount) + j].z = zStart + j * dz;
		}
	}
}
