#include "QuadSystem.h"

std::vector<int> QuadSystem::triangulate()
{
	std::vector<int> triIndexList;
	for (int i = 0; i < quads.size(); i++)
	{
		for (int j = 0; j < 6; j++)
		{
			triIndexList.push_back(i * 6 + triIndices[j]);
		}
	}
	return triIndexList;
}

// here we populate verts and tris, all other initialization happens in Renderer class
std::unique_ptr<Mesh> QuadSystem::convertToMesh1()
{
	// standard indexing
	std::vector<int> triangles = triangulate();
	std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>();
	mesh->initMesh(quads.size() * 4, triangles.size());
	for (int i = 0; i < quads.size(); i++)
	{
		Vertex v;
		v.assign(quads[i].v1.x, quads[i].v1.y, quads[i].v1.z);
		mesh->vertices[i * 4 + 0] = v;
		v.assign(quads[i].v2.x, quads[i].v2.y, quads[i].v2.z);
		mesh->vertices[i * 4 + 1] = v;
		v.assign(quads[i].v3.x, quads[i].v3.y, quads[i].v3.z);
		mesh->vertices[i * 4 + 2] = v;
		v.assign(quads[i].v4.x, quads[i].v4.y, quads[i].v4.z);
		mesh->vertices[i * 4 + 3] = v;
	}
	for (int i = 0; i < triangles.size(); i++)
	{
		mesh->tris[i] = triangles[i];
	}
	return mesh;
}


void QuadSystem::buildFlatSquare(float length)
{
	Quad square;
	// -1, 0, -1
	square.v1.x = origin.x - length / 2;
	square.v1.y = 0.0f;
	square.v1.z = origin.z - length / 2;
	// 1, 0, -1
	square.v2.x = origin.x + length / 2;
	square.v2.y = 0.0f;
	square.v2.z = origin.z - length / 2;
	// 1, 0, 1
	square.v3.x = origin.x + length / 2;
	square.v3.y = 0.0f;
	square.v3.z = origin.z + length / 2;
	// -1, 0 , 1
	square.v4.x = origin.x - length / 2;
	square.v4.y = 0.0f;
	square.v4.z = origin.z + length / 2;

	quads.push_back(square);
}

