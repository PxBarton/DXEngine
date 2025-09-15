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

std::vector<XMFLOAT3> QuadSystem::scalePolygon(std::vector<XMFLOAT3> verts, XMFLOAT3 axisPoint, XMFLOAT3 scale) {
	// translate, scale, translate
	XMMATRIX toOrigin = XMMatrixTranslation(-axisPoint.x, -axisPoint.y, -axisPoint.z);
	XMMATRIX S = XMMatrixScaling(scale.x, scale.y, scale.z);
	XMMATRIX fromOrigin = XMMatrixTranslation(axisPoint.x, axisPoint.y, axisPoint.z);

	XMMATRIX combinedTransform = XMMatrixMultiply(XMMatrixMultiply(toOrigin, S), fromOrigin);

	std::vector<XMFLOAT3> transformedPoly;
	// apply the combined transform to each point
	for (const XMFLOAT3& v : verts) {
		// Convert XMFLOAT3 to XMVECTOR for transform
		XMVECTOR vertVec = XMLoadFloat3(&v);

		XMVECTOR v_transformed = XMVector3Transform(vertVec, combinedTransform);

		// back to XMFLOAT3
		XMFLOAT3 transformed; 
		XMStoreFloat3(&transformed, v_transformed);
		transformedPoly.push_back(transformed);
	}

	return transformedPoly;
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

void QuadSystem::buildBox(std::array<int, 4> nearCorners, XMFLOAT3 normal, float length, XMFLOAT3 endScale)
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

	//std::array<XMFLOAT3, 4> newCorners = { farCorner0 , farCorner1, farCorner2, farCorner3 };
	std::vector<XMFLOAT3> newCorners = { farCorner0 , farCorner1, farCorner2, farCorner3 };
	
	XMFLOAT3 scaleOrigin = { 0.0f, 0.0f, 0.0f };
	for (const XMFLOAT3 vert : newCorners) {
		scaleOrigin.x += vert.x;
		scaleOrigin.y += vert.y;
		scaleOrigin.z += vert.z;
	}

	// assume quads for now
	scaleOrigin.x /= 4;
	scaleOrigin.y /= 4;
	scaleOrigin.z /= 4;

	std::vector<XMFLOAT3> farCorners = scalePolygon(newCorners, scaleOrigin, endScale);

	Box box;

	box.verts = { nearCorners[0],
				nearCorners[1],
				nearCorners[2],
				nearCorners[3],
				pointCount() + 0,
				pointCount() + 1,
				pointCount() + 2,
				pointCount() + 3};


	points.push_back(farCorners[0]);
	points.push_back(farCorners[1]);
	points.push_back(farCorners[2]);
	points.push_back(farCorners[3]);

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

void QuadSystem::topCap(Box& box)
{
	std::array<int, 4> corners = box.farCorners();
	Face cap;
	cap.face.push_back(corners[0]);
	cap.face.push_back(corners[1]);
	cap.face.push_back(corners[2]);
	cap.face.push_back(corners[3]);

	faces.push_back(cap);
}

void QuadSystem::bottomCap(Box& box)
{
	std::array<int, 4> corners = box.nearCorners();
	Face cap;
	cap.face.push_back(corners[3]);
	cap.face.push_back(corners[2]);
	cap.face.push_back(corners[1]);
	cap.face.push_back(corners[0]);

	faces.push_back(cap);
}

void QuadSystem::findEdges()
{
	faceEdgePairs.clear();
	std::set<std::pair<int, int>> uniqueEdgesSet;

	// loop through each face
	for (const Face& f : faces) {
		// Get the number of vertices in this face
		size_t s = f.face.size();
		std::vector<std::pair<int, int>> faceEdges;

		// loop through each edge on the face
		for (size_t j = 0; j < s; ++j) {
			// two point indices for the current edge
			// modulo wrapping around
			int v1 = f.face[j];
			int v2 = f.face[(j + 1) % s];

			// ensure first index is always smaller than the second.
			// (5, 10) is the same as (10, 5).
			if (v1 > v2) {
				std::swap(v1, v2);
			}
			std::pair<int, int> uniqueEdge = { v1, v2 };
			// set prevents duplicates.
			faceEdges.push_back(uniqueEdge);
			uniqueEdgesSet.insert(uniqueEdge);
		}
		faceEdgePairs.push_back(faceEdges);
	}
	this->edges.clear();

	this->edges.assign(uniqueEdgesSet.begin(), uniqueEdgesSet.end());
}

void QuadSystem::CCsubdivide()
{
	// build edge list
	findEdges();

	// adjacent faces to points
	// map setup for Face objects, vector<int> might be better
	std::unordered_map<int, std::vector<Face>> PointsToFacesMap;
	std::unordered_map<int, std::array<int, 2>> edgeToFacesMap;
	std::unordered_map<int, std::vector<int>> faceToEdgesMap;
	
	
	// slow
	for (int p = 0; p < points.size(); p++)
	{
		std::vector<Face> adjFaces;
		for (int f = 0; f < faces.size(); f++)
		{
			// check if the point index exists in the face array
			for (int i : faces[f].face)
			{
				if (p == i)
				{
					adjFaces.push_back(faces[f]);
					break;
				}
					
			}
		}
	}

	// faster, hash lookup, no order
	for (const Face& face : faces) {
		for (int p_index : face.face) {
			PointsToFacesMap[p_index].push_back(face);
		}
	}

	for (int e_idx = 0; e_idx < edges.size(); ++e_idx) {
		const auto& edge = edges[e_idx];

		// Replicates the Python 'adjFaces = []' logic
		std::array<int, 2> adjFaces = { -1, -1 };
		int faceCount = 0;

		// Replicates the Python 'for i in range(len(eList))' and 'if edges[e] in eList[i]'
		for (int f_idx = 0; f_idx < faceEdgePairs.size(); ++f_idx) {
			// std::find is the C++ equivalent of Python's 'in' for a list
			if (std::find(faceEdgePairs[f_idx].begin(), faceEdgePairs[f_idx].end(), edge) != faceEdgePairs[f_idx].end()) {
				if (faceCount < 2) {
					adjFaces[faceCount] = f_idx;
					faceCount++;
				}
				// We can stop searching for this edge if we've found both faces
				if (faceCount == 2) {
					break;
				}
			}
		}
		edgeToFacesMap[e_idx] = adjFaces;
	}

	for (int f_idx = 0; f_idx < faceEdgePairs.size(); ++f_idx) {
		std::vector<int> adjEdges;
		for (const auto& faceEdge : faceEdgePairs[f_idx]) {
			// This is the slow part: finding the index of an edge in the uniqueEdges list
			// This is needed because the map key is an int index.
			auto it = std::find(edges.begin(), edges.end(), faceEdge);
			if (it != edges.end()) {
				int edge_idx = std::distance(edges.begin(), it);
				adjEdges.push_back(edge_idx);
			}
		}
		faceToEdgesMap[f_idx] = adjEdges;
	}

}



