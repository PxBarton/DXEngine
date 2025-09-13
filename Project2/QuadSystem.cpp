#include "QuadSystem.h"

std::vector<int> QuadSystem::triangulateQuads()
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

std::vector<int> QuadSystem::triangulateFaces()
{
	// assume quads for now
	std::vector<int> triIndexList;
	for (int f = 0; f < faces.size(); f++)
	{
		for (int t = 0; t < triIndices.size(); t++)
		{
			int vertIndex = triIndices[t];
			triIndexList.push_back(faces[f].face[vertIndex]);
		}
	}	
	
	return triIndexList;
}

// here we populate verts and tris from Quads, all other initialization happens in Renderer class
std::unique_ptr<Mesh> QuadSystem::convertQuadsToMesh()
{
	// standard indexing
	std::vector<int> triangles = triangulateQuads();
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

// populate verts and tris from Faces
std::unique_ptr<Mesh> QuadSystem::convertFacesToMesh()
{
	// standard indexing
	std::vector<int> triangles = triangulateFaces();
	std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>();
	mesh->initMesh(points.size(), triangles.size());
	for (int i = 0; i < points.size(); i++)
	{
		Vertex v;
		v.assign(points[i].x, points[i].y, points[i].z);
		mesh->vertices[i] = v;
	}
	for (int i = 0; i < triangles.size(); i++)
	{
		mesh->tris[i] = triangles[i];
	}
	return mesh;
}

void QuadSystem::addPoints(std::vector<XMFLOAT3> newPoints)
{
	for (int i = 0; i < newPoints.size(); i++)
	{
		points.push_back(newPoints[i]);
	}
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

void QuadSystem::buildBox(XMFLOAT3 start, XMFLOAT3 normal, float radius1, float radius2, float length)
{

}

void QuadSystem::buildBox(std::array<int, 4> nearCorners, XMFLOAT3 normal, float length, float taper)
{
	XMFLOAT3 axisVector = XMFLOAT3(normal.x * length, normal.y * length, normal.z * length);

	float farCorner0x = points[nearCorners[0]].x + axisVector.x;
	float farCorner0y = points[nearCorners[0]].y + axisVector.y;
	float farCorner0z = points[nearCorners[0]].z + axisVector.z;
	XMFLOAT3 farCorner0 = XMFLOAT3(farCorner0x, farCorner0y, farCorner0z);

	float farCorner1x = points[nearCorners[1]].x + axisVector.x;
	float farCorner1y = points[nearCorners[1]].y + axisVector.y;
	float farCorner1z = points[nearCorners[1]].z + axisVector.z;
	XMFLOAT3 farCorner1 = XMFLOAT3(farCorner1x, farCorner1y, farCorner1z);

	float farCorner2x = points[nearCorners[2]].x + axisVector.x;
	float farCorner2y = points[nearCorners[2]].y + axisVector.y;
	float farCorner2z = points[nearCorners[2]].z + axisVector.z;
	XMFLOAT3 farCorner2 = XMFLOAT3(farCorner2x, farCorner2y, farCorner2z);

	float farCorner3x = points[nearCorners[3]].x + axisVector.x;
	float farCorner3y = points[nearCorners[3]].y + axisVector.y;
	float farCorner3z = points[nearCorners[3]].z + axisVector.z;
	XMFLOAT3 farCorner3 = XMFLOAT3(farCorner3x, farCorner3y, farCorner3z);

	Box box;

	box.verts = { nearCorners[0],
				nearCorners[1],
				nearCorners[2],
				nearCorners[3],
				pointCount() + 0,
				pointCount() + 1,
				pointCount() + 2,
				pointCount() + 3};


	points.push_back(farCorner0);
	points.push_back(farCorner1);
	points.push_back(farCorner2);
	points.push_back(farCorner3);

	// box.verts[face[i]] 
	// ultimately need to reference the class points list to avoid confusion and repeated verts
	Face side0;
	//side0.face = { 0, 4, 5, 1 };
	side0.face = { box.verts[0], box.verts[4], box.verts[5], box.verts[1] };

	Face side1;
	//side1.face = { 1, 5, 6, 2 };
	side1.face = { box.verts[1], box.verts[5], box.verts[6], box.verts[2] };

	Face side2;
	//side2.face = { 2, 6, 7, 3 };
	side2.face = { box.verts[2], box.verts[6], box.verts[7], box.verts[3] };

	Face side3;
	//side3.face = { 3, 7, 4, 0 };
	side3.face = { box.verts[3], box.verts[7], box.verts[4], box.verts[0] };

	box.faces = { side0, side1, side2, side3 };
	faces.push_back(side0);
	faces.push_back(side1);
	faces.push_back(side2);
	faces.push_back(side3);

	box.direction = normal;

	boxes.push_back(box);
}



