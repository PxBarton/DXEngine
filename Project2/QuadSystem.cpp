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
		if (faceExists(f))
		{
			for (int t = 0; t < triIndices.size(); t++)
			{
				int vertIndex = triIndices[t];
				triIndexList.push_back(faces[f].face[vertIndex]);
			}
		}
		
	}	
	
	return triIndexList;
}

XMVECTOR QuadSystem::calcNormal(int p1, int p2, int p3)
{
	XMVECTOR vA = DirectX::XMLoadFloat3(&points[p1]);
	XMVECTOR vB = DirectX::XMLoadFloat3(&points[p2]);
	XMVECTOR vC = DirectX::XMLoadFloat3(&points[p3]);

	XMVECTOR s = DirectX::XMVectorSubtract(vB, vA);
	XMVECTOR t = DirectX::XMVectorSubtract(vC, vA);

	DirectX::XMVECTOR normalV = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(s, t));
	return (normalV);
}

XMVECTOR QuadSystem::calcNormal(int faceIndex)
{
	int p1 = faces[faceIndex].face[0];
	int p2 = faces[faceIndex].face[1];
	int p3 = faces[faceIndex].face[2];
	XMVECTOR vA = DirectX::XMLoadFloat3(&points[p1]);
	XMVECTOR vB = DirectX::XMLoadFloat3(&points[p2]);
	XMVECTOR vC = DirectX::XMLoadFloat3(&points[p3]);

	XMVECTOR s = DirectX::XMVectorSubtract(vB, vA);
	XMVECTOR t = DirectX::XMVectorSubtract(vC, vA);

	DirectX::XMVECTOR normalV = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(s, t));
	return (normalV);
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
		DirectX::XMStoreFloat3(&transformed, v_transformed);
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

	box.faceIndices.push_back(faceCount());
	faces.push_back(side0);
	box.faceIndices.push_back(faceCount());
	faces.push_back(side1);
	box.faceIndices.push_back(faceCount());
	faces.push_back(side2);
	box.faceIndices.push_back(faceCount());
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

void QuadSystem::replaceFace(int faceIndex, XMFLOAT3 normal, float length, XMFLOAT3 endScale, bool cap)
{
	std::vector<int> faceToReplace = faces[faceIndex].face;
	std::array<int, 4> oldFace;
	oldFace[0] = faceToReplace[0];
	oldFace[1] = faceToReplace[1];
	oldFace[2] = faceToReplace[2];
	oldFace[3] = faceToReplace[3];

	buildBox(oldFace, normal, length, endScale);
	if (cap)
	{
		topCap(boxes[boxes.size() - 1]);
	}

	//faces.erase(faces.begin() + faceIndex-1);
	//faceDeletionList.push_back(faceIndex);
	//faces[faceIndex].face[0] = -1;
}

void QuadSystem::deleteFaces()
{
	for (int index : faceDeletionList)
	{
		faces[index].face[0] = -99;
	}
}

XMFLOAT3 QuadSystem::edgeMidpoint(const XMFLOAT3& p1, const XMFLOAT3& p2) {
	XMVECTOR v1 = XMLoadFloat3(&p1);
	XMVECTOR v2 = XMLoadFloat3(&p2);
	XMVECTOR midpoint = XMVectorScale(XMVectorAdd(v1, v2), 0.5f);
	XMFLOAT3 result;
	DirectX::XMStoreFloat3(&result, midpoint);
	return result;
}

// Returns a new XMFLOAT3 that is the centroid of a list of vertices
XMFLOAT3 QuadSystem::findCentroid(const std::vector<XMFLOAT3>& points) {
	XMVECTOR sum = XMVectorZero();
	for (const auto& point : points) {
		sum = XMVectorAdd(sum, XMLoadFloat3(&point));
	}
	XMVECTOR centroid = XMVectorScale(sum, 1.0f / static_cast<float>(points.size()));
	XMFLOAT3 result;
	DirectX::XMStoreFloat3(&result, centroid);
	return result;
}

void QuadSystem::findEdges()
{
	faceEdgePairs.clear();
	std::set<std::pair<int, int>> uniqueEdgesSet;

	// loop through each face
	for (const Face& f : faces) {
		if (faceExists(f))
		{
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
		
	}
	this->edges.clear();

	this->edges.assign(uniqueEdgesSet.begin(), uniqueEdgesSet.end());
}

void QuadSystem::CCsubdivide(float paramA = 3.0, float paramB = 2.0, float paramC = 1.0)
{
	// phase 1: build adjacency lists
	
	// build edge list
	findEdges();

	// map setup for Face objects, vector<int> might be better
	std::unordered_map<int, std::vector<int>> pointToFacesMap;
	std::unordered_map<int, std::array<int, 2>> edgeToFacesMap;
	std::unordered_map<int, std::vector<int>> faceToEdgesMap;
	std::unordered_map<int, std::vector<int>> pointToEdgesMap;
	
	// point to faces
	// slow
	/*
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
	*/

	// point to faces
	// faster, hash lookup, no order
	//for (const Face& face : faces) {
	for (int f_idx = 0; f_idx < faces.size(); ++f_idx) {
		if (faceExists(f_idx))
		{
			const Face& face = faces[f_idx];
			for (int p_index : face.face) {
				//pointToFacesMap[p_index].push_back(face);
				pointToFacesMap[p_index].push_back(f_idx);
			}
		}
		
	}


	// edge to faces
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

	// face to edges
	for (int f = 0; f < faceEdgePairs.size(); ++f) {
		std::vector<int> adjEdges;
		for (const auto& faceEdge : faceEdgePairs[f]) {
			// This is the slow part: finding the index of an edge in the uniqueEdges list
			// This is needed because the map key is an int index.
			auto it = std::find(edges.begin(), edges.end(), faceEdge);
			if (it != edges.end()) {
				int edge_idx = std::distance(edges.begin(), it);
				adjEdges.push_back(edge_idx);
			}
		}
		faceToEdgesMap[f] = adjEdges;
	}

	// point to edges
	// Loop through each vertex in the mesh
	for (int v = 0; v < points.size(); ++v) {
		std::vector<int> adjEdges;
		// This is the slow part: a brute-force search
		for (int e = 0; e < edges.size(); ++e) {
			const auto& edge = edges[e];
			// Check if the vertex index is in the edge pair
			if (edge.first == v || edge.second == v) {
				adjEdges.push_back(e);
			}
		}
		pointToEdgesMap[v] = adjEdges;
	}

	// phase 2: calculate new geometry

	std::unordered_map<int, XMFLOAT3> facePoints;
	std::unordered_map<int, XMFLOAT3> edgePoints;
	std::unordered_map<int, XMFLOAT3> vertPoints;

	// calculate face points 
	facePoints.clear();
	for (int f_idx = 0; f_idx < faces.size(); ++f_idx) {
		if (faceExists(f_idx))
		{
			std::vector<XMFLOAT3> faceVerts;
			for (int v_idx : faces[f_idx].face) {
				faceVerts.push_back(points[v_idx]);
			}
			facePoints[f_idx] = findCentroid(faceVerts);
		}
		
	}

	// calculate edge points 
	edgePoints.clear();
	for (int e_idx = 0; e_idx < edges.size(); ++e_idx) {
		const auto& edge = edges[e_idx];

		// Edge midpoint (P in the algorithm)
		XMFLOAT3 edgeMid = edgeMidpoint(points[edge.first], points[edge.second]);

		// Average of adjacent face points (R in the algorithm)
		XMFLOAT3 fp1 = facePoints[edgeToFacesMap[e_idx][0]];
		XMFLOAT3 fp2 = facePoints[edgeToFacesMap[e_idx][1]];
		XMFLOAT3 avgFP = edgeMidpoint(fp1, fp2);

		// New edge point is the average of P and R
		XMFLOAT3 newEdgePoint;
		XMVECTOR v_new_edge = XMVectorScale(XMVectorAdd(XMLoadFloat3(&edgeMid), XMLoadFloat3(&avgFP)), 0.5f);
		DirectX::XMStoreFloat3(&newEdgePoint, v_new_edge);

		edgePoints[e_idx] = newEdgePoint;
	}

	// calculate vertex points
	vertPoints.clear();
	for (int v_idx = 0; v_idx < points.size(); ++v_idx) {
		// Find average of adjacent face points (F)
		std::vector<XMFLOAT3> adjFacePoints;
		const auto& adjFaces = pointToFacesMap[v_idx];
		for (int f_idx : adjFaces) {
			adjFacePoints.push_back(facePoints[f_idx]);
		}
		XMFLOAT3 F = findCentroid(adjFacePoints);

		// Find average of adjacent edge midpoints (R)
		std::vector<XMFLOAT3> adjEdgeMids;
		for (int e_idx : pointToEdgesMap[v_idx]) {
			const auto& edge = edges[e_idx];
			adjEdgeMids.push_back(edgeMidpoint(points[edge.first], points[edge.second]));
		}
		XMFLOAT3 R = findCentroid(adjEdgeMids);

		// Original vertex point (P)
		XMFLOAT3 P = points[v_idx];

		// n is the number of faces/edges adjacent to the vertex. This number is dynamic.
		int n = adjFaces.size(); // CORRECTED: Get n from the new map's size

		// Catmull-Clark formula for the new vertex point
		float m1 = (static_cast<float>(n) - paramA) / static_cast<float>(n);
		float m2 = paramB / static_cast<float>(n);
		float m3 = paramC / static_cast<float>(n);

		XMVECTOR v_new_vert = XMVectorAdd(
			XMVectorScale(XMLoadFloat3(&P), m1),
			XMVectorAdd(
				XMVectorScale(XMLoadFloat3(&F), m2),
				XMVectorScale(XMLoadFloat3(&R), m3)
			)
		);

		XMFLOAT3 newVertPoint;
		DirectX::XMStoreFloat3(&newVertPoint, v_new_vert);
		vertPoints[v_idx] = newVertPoint;
	}

	// --- Step 5: Construct the new mesh ---
	std::vector<XMFLOAT3> newPoints;
	std::vector<Face> newFaces;

	// Add new vertex points
	for (int i = 0; i < points.size(); ++i) {
		newPoints.push_back(vertPoints[i]);
	}
	// Add new edge points
	for (int i = 0; i < edges.size(); ++i) {
		newPoints.push_back(edgePoints[i]);
	}
	// Add new face points
	for (int i = 0; i < faces.size(); ++i) {
		newPoints.push_back(facePoints[i]);
	}

	// Construct new faces (Quads)
	size_t n1 = points.size();
	size_t n2 = edges.size();

	for (int f_idx = 0; f_idx < faces.size(); ++f_idx) {
		if (faceExists(f_idx))
		{
			const auto& oldFace = faces[f_idx];
			const auto& oldEdges = faceToEdgesMap[f_idx];

			// A temporary vector to hold the new faces for this old face
			std::vector<Face> subFaces;

			for (int i = 0; i < oldFace.face.size(); ++i) {
				Face newFace;
				// Original vertex point (V')
				newFace.face.push_back(oldFace.face[i]);
				// Edge point of the next edge (E')
				newFace.face.push_back(n1 + faceToEdgesMap[f_idx][i]);
				// Face point (F')
				newFace.face.push_back(n1 + n2 + f_idx);
				// Edge point of the previous edge (E'')
				newFace.face.push_back(n1 + faceToEdgesMap[f_idx][(i + 3) % 4]);

				subFaces.push_back(newFace);
			}
			newFaces.insert(newFaces.end(), subFaces.begin(), subFaces.end());
		}
		
	}

	// Replace the old mesh with the new one
	points = newPoints;
	faces = newFaces;

}

// euclidean distance
float QuadSystem::distance(XMFLOAT3 pt1, XMFLOAT3 pt2)
{
	float X = abs(pt1.x - pt2.x);
	float Y = abs(pt1.y - pt2.y);
	float Z = abs(pt1.z - pt2.z);
	return sqrt(X * X + Y * Y + Z * Z);
}

float QuadSystem::approxWidth(Face f)
{
	XMFLOAT3 v0 = points[f.face[0]];
	XMFLOAT3 v1 = points[f.face[1]];
	XMFLOAT3 v2 = points[f.face[2]];
	XMFLOAT3 v3 = points[f.face[3]];
	float diagonal1 = distance(v0, v2);
	float diagonal2 = distance(v1, v3);
	
	return (diagonal1 + diagonal2) / 2.0;
}

// angle away from initial face normal
void QuadSystem::branch(int faceIndex, std::vector<int> sides, float split, std::vector<float> angles, float widthPercent, float boxHeightRatio)
{
	Face& f = faces[faceIndex];
	float newBoxHeight = approxWidth(faces[faceIndex])* 0.8 * boxHeightRatio;
	XMFLOAT3 scale = XMFLOAT3(0.6, 0.6, 0.6);
	XMFLOAT3 boxNormal;
	// not sure why calcNormal gets the sign wrong 
	XMVECTOR boxNormalV = -calcNormal(faceIndex);
	XMStoreFloat3(&boxNormal, boxNormalV);
	replaceFace(faceIndex, boxNormal, newBoxHeight, scale, true);
	Box newBox = getBox(boxCount() - 1);
	for (int s = 0; s < sides.size(); s++)
	{
		int face = newBox.faceIndices[sides[s]];
		XMFLOAT3 newNormal = rotateVector(boxNormalV, calcNormal(face), angles[s]);
		replaceFace(face, newNormal, newBoxHeight * 4, scale, true);
	}
	// dont forget to store indices of caps to make more branches

}



