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
		if (faces[f].face.size() == 4)
		{
			for (int t = 0; t < triIndices.size(); t++)
			{
				int vertIndex = triIndices[t];
				triIndexList.push_back(faces[f].face[vertIndex]);
			}
		}
		if (faces[f].face.size() == 3)
		{
			for (int v = 0; v < 3; v++)
			{
				triIndexList.push_back(faces[f].face[v]);
			}
		}
		
	}	
	
	return triIndexList;
}

void QuadSystem::triangulateNgon(uint64_t faceId)
{
	Face f = getFaceById(faceId);
	// fan triangulation
	int numVerts = f.face.size();
	std::vector<XMFLOAT3> faceVerts;
	for (const auto& vert : f.face)
	{
		faceVerts.push_back(points[vert]);
	}
	XMFLOAT3 centroid = findNgonCentroid(faceVerts);
	points.push_back(centroid);
	int centroidIndex = points.size() - 1;
	for (int i = 0; i < numVerts; i++)
	{
		Face f = getFaceById(faceId);
		Face triFace;
		triFace.face.push_back(centroidIndex);
		triFace.face.push_back(f.face[(i + 1) % numVerts]);
		triFace.face.push_back(f.face[i]);
		
		
		faces.push_back(triFace);
		trackNewFace(faces.back(), faceCount() - 1);
	}
	faceDeletionIdList.push_back(faceId);
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

void QuadSystem::assignNormal(Face& f)
{
	int p1 = f.face[0];
	int p2 = f.face[1];
	int p3 = f.face[2];
	XMVECTOR vA = DirectX::XMLoadFloat3(&points[p1]);
	XMVECTOR vB = DirectX::XMLoadFloat3(&points[p2]);
	XMVECTOR vC = DirectX::XMLoadFloat3(&points[p3]);

	XMVECTOR s = DirectX::XMVectorSubtract(vB, vA);
	XMVECTOR t = DirectX::XMVectorSubtract(vC, vA);

	f.normalV = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(s, t));
	XMStoreFloat3(&f.normal, f.normalV);
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
	XMVECTOR normalV = DirectX::XMLoadFloat3(&normal);
	normalV = DirectX::XMVector3Normalize(normalV);
	XMFLOAT3 axisVector = XMFLOAT3(normal.x * length, normal.y * length, normal.z * length);
	XMFLOAT3 center = findCentroid(nearCorners);

	XMVECTOR axisVectorV = DirectX::XMVectorScale(normalV, length);
	XMVECTOR centerV = DirectX::XMLoadFloat3(&center);
	XMVECTOR endPointV = DirectX::XMVectorAdd(centerV, axisVectorV);
	XMFLOAT3 endPoint;
	DirectX::XMStoreFloat3(&endPoint, endPointV);
	
	XMVECTOR P1 = XMLoadFloat3(&points[nearCorners[0]]);
	XMVECTOR P2 = XMLoadFloat3(&points[nearCorners[1]]);
	XMVECTOR P3 = XMLoadFloat3(&points[nearCorners[2]]);
	XMVECTOR P4 = XMLoadFloat3(&points[nearCorners[3]]);

	std::array<XMVECTOR, 4> nearCornersV = { P1, P2, P3, P4 };

	std::array<XMVECTOR, 4> projectedPointsV = projectQuad(P1, P2, P3, P4, DirectX::XMLoadFloat3(&normal), length);
	//std::vector<XMVECTOR> projectedPointsV = projectQuad(nearCornersV, normalV, length);
	std::vector<XMFLOAT3> projectedPoints;

	for (size_t i = 0; i < projectedPointsV.size(); ++i) {
		XMFLOAT3 point;
		DirectX::XMStoreFloat3(&point, projectedPointsV[i]);
		projectedPoints.push_back(point);
	}

	XMFLOAT3 farCenter = findNgonCentroid(projectedPoints);

	//std::vector<XMFLOAT3> farCorners = scalePolygon(newCorners, scaleOrigin, endScale);
	std::vector<XMFLOAT3> farCorners = scalePolygon(projectedPoints, farCenter, endScale);

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

	

	//box.faces = { side0, side1, side2, side3 };

	box.faceIndices.push_back(faceCount());
	faces.push_back(side0);
	trackNewFace(faces.back(), faceCount() - 1);
	box.faceIds.push_back(faces.back().id);
	assignNormal(side0);

	box.faceIndices.push_back(faceCount());
	faces.push_back(side1);
	trackNewFace(faces.back(), faceCount() - 1);
	box.faceIds.push_back(faces.back().id); 
	assignNormal(side1);

	box.faceIndices.push_back(faceCount());
	faces.push_back(side2);
	trackNewFace(faces.back(), faceCount() - 1);
	box.faceIds.push_back(faces.back().id);
	assignNormal(side2);

	box.faceIndices.push_back(faceCount());
	faces.push_back(side3);
	trackNewFace(faces.back(), faceCount() - 1);
	box.faceIds.push_back(faces.back().id);
	assignNormal(side3);

	box.direction = normal;

	boxes.push_back(box);
}

uint64_t QuadSystem::topCap(Box& box)
{
	std::array<int, 4> corners = box.farCorners();
	Face cap;
	cap.face.push_back(corners[0]);
	cap.face.push_back(corners[3]);
	cap.face.push_back(corners[2]);
	cap.face.push_back(corners[1]);
	assignNormal(cap);

	faces.push_back(cap);
	trackNewFace(faces.back(), faceCount() - 1);
	uint64_t capId = faces.back().id;
	topCapId = capId;
	//box.faceIds.push_back(faces.back().id);

	return capId;
}

uint64_t QuadSystem::topCap(Box& box, bool polarity)
{
	std::array<int, 4> corners = box.farCorners();
	Face cap;

	if (polarity)
	{
		cap.face.push_back(corners[0]);
		cap.face.push_back(corners[1]);
		cap.face.push_back(corners[2]);
		cap.face.push_back(corners[3]);
	}
	else
	{
		cap.face.push_back(corners[0]);
		cap.face.push_back(corners[3]);
		cap.face.push_back(corners[2]);
		cap.face.push_back(corners[1]);
	}
	assignNormal(cap);

	faces.push_back(cap);
	trackNewFace(faces.back(), faceCount() - 1);
	uint64_t capId = faces.back().id;
	topCapId = capId;
	//box.faceIds.push_back(faces.back().id);

	return capId;
}

uint64_t QuadSystem::addCap(Box& box)
{
	std::array<int, 4> corners = box.farCorners();
	Face cap;
	cap.face.push_back(corners[0]);
	cap.face.push_back(corners[3]);
	cap.face.push_back(corners[2]);
	cap.face.push_back(corners[1]);
	assignNormal(cap);

	faces.push_back(cap);
	trackNewFace(faces.back(), faceCount() - 1);
	uint64_t capId = faces.back().id;

	return capId;
}

uint64_t QuadSystem::addCap(Box& box, bool polarity)
{
	std::array<int, 4> corners = box.farCorners();
	Face cap;

	if (polarity)
	{
		cap.face.push_back(corners[0]);
		cap.face.push_back(corners[1]);
		cap.face.push_back(corners[2]);
		cap.face.push_back(corners[3]);
	}
	else
	{
		cap.face.push_back(corners[0]);
		cap.face.push_back(corners[3]);
		cap.face.push_back(corners[2]);
		cap.face.push_back(corners[1]);
	}
	assignNormal(cap);

	faces.push_back(cap);
	trackNewFace(faces.back(), faceCount() - 1);
	uint64_t capId = faces.back().id;

	return capId;
}

void QuadSystem::bottomCap(Box& box)
{
	std::array<int, 4> corners = box.nearCorners();
	Face cap;
	cap.face.push_back(corners[1]);
	cap.face.push_back(corners[2]);
	cap.face.push_back(corners[3]);
	cap.face.push_back(corners[0]);

	faces.push_back(cap);
	trackNewFace(faces.back(), faceCount() - 1);
	bottomCapId = faces.back().id;
	//box.faceIds.push_back(faces.back().id);
}

void QuadSystem::bottomCap(Box& box, bool polarity)
{
	std::array<int, 4> corners = box.nearCorners();
	Face cap;
	if (polarity)
	{
		cap.face.push_back(corners[0]);
		cap.face.push_back(corners[1]);
		cap.face.push_back(corners[2]);
		cap.face.push_back(corners[3]);
	}
	else
	{
		cap.face.push_back(corners[1]);
		cap.face.push_back(corners[2]);
		cap.face.push_back(corners[3]);
		cap.face.push_back(corners[0]);
	}	

	faces.push_back(cap);
	trackNewFace(faces.back(), faceCount() - 1);
	bottomCapId = faces.back().id;
	//box.faceIds.push_back(faces.back().id);
}
/*
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
}
*/
// prioritize unique id's over indices that may change with deletions
void QuadSystem::replaceFace(uint64_t faceId, XMFLOAT3 normal, float length, XMFLOAT3 endScale, bool cap, bool isBranch)
{
	int faceIndex = getFaceIndexByID(faceId);
	std::vector<int> faceToReplace = faces[faceIndex].face;
	std::array<int, 4> oldFace;
	oldFace[0] = faceToReplace[0];
	oldFace[1] = faceToReplace[1];
	oldFace[2] = faceToReplace[2];
	oldFace[3] = faceToReplace[3];

	buildBox(oldFace, normal, length, endScale);
	uint64_t capId;
	if (cap)
	{
		capId = addCap(boxes[boxes.size() - 1]);
		capIds.push_back(capId);
	}

	faceDeletionIdList.push_back(faceId);

	if (isBranch && cap)
	{
		branchCaps.push_back(capId);
	}
}

void QuadSystem::replaceFace(uint64_t faceId, XMFLOAT3 normal, float length, XMFLOAT3 endScale, bool polarity, bool cap, bool isBranch)
{
	int faceIndex = getFaceIndexByID(faceId);
	std::vector<int> faceToReplace = faces[faceIndex].face;
	std::array<int, 4> oldFace;
	oldFace[0] = faceToReplace[0];
	oldFace[1] = faceToReplace[1];
	oldFace[2] = faceToReplace[2];
	oldFace[3] = faceToReplace[3];

	buildBox(oldFace, normal, length, endScale);
	uint64_t capId;
	if (cap)
	{
		capId = addCap(boxes[boxes.size() - 1]);
		capIds.push_back(capId);
	}

	faceDeletionIdList.push_back(faceId);

	if (isBranch && cap)
	{
		branchCaps.push_back(capId);
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


XMFLOAT3 QuadSystem::findCentroid(const std::array<int, 4>& vertices) {
	std::array<XMFLOAT3, 4> facePoints = { points[vertices[0]], points[vertices[1]], points[vertices[2]], points[vertices[3]] };
	XMVECTOR sum = XMVectorZero();
	for (const auto& point : facePoints) {
		sum = XMVectorAdd(sum, XMLoadFloat3(&point));
	}
	XMVECTOR centroid = XMVectorScale(sum, 1.0f / static_cast<float>(points.size()));
	XMFLOAT3 result;
	DirectX::XMStoreFloat3(&result, centroid);
	return result;
}

XMVECTOR QuadSystem::findCentroidV(const std::array<XMVECTOR, 4>& vertices) {
	XMVECTOR sum = XMVectorZero();
	for (const auto& point : vertices) {
		sum = XMVectorAdd(sum, point);
	}
	XMVECTOR centroid = XMVectorScale(sum, 1.0f / static_cast<float>(vertices.size()));
	return centroid;
}

// Returns a new XMFLOAT3 that is the centroid of a list of vertices
XMFLOAT3 QuadSystem::findNgonCentroid(const std::vector<XMFLOAT3>& vertices) {
	XMVECTOR sum = XMVectorZero();
	for (const auto& point : vertices) {
		sum = XMVectorAdd(sum, XMLoadFloat3(&point));
	}
	XMVECTOR centroid = XMVectorScale(sum, 1.0f / static_cast<float>(vertices.size()));
	XMFLOAT3 result;
	DirectX::XMStoreFloat3(&result, centroid);
	return result;
}


XMVECTOR QuadSystem::findNgonCentroidV(const std::vector<XMVECTOR>& vertices) {
	XMVECTOR sum = XMVectorZero();
	for (const auto& point : vertices) {
		sum = XMVectorAdd(sum, point);
	}
	XMVECTOR centroid = XMVectorScale(sum, 1.0f / static_cast<float>(vertices.size()));
	return centroid;
}

// populates edges and faceEdgePairs at the same time
void QuadSystem::findEdges()
{
	faceEdgePairs.clear();
	std::set<std::pair<int, int>> uniqueEdgesSet;

	// loop through each face
	for (const Face& f : faces) {
		if (true)
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
int QuadSystem::branch(Branch& parent, int faceId, std::vector<int> sides, std::vector<float> angles, float boxHeightRatio)
{
	int faceIndex = getFaceIndexByID(faceId);
	Face& f = faces[faceIndex];
	float newBoxHeight = approxWidth(faces[faceIndex])* 0.8 * boxHeightRatio;
	XMFLOAT3 scale = XMFLOAT3(0.6, 0.6, 0.6);
	XMFLOAT3 boxNormal;
	// not sure why calcNormal gets the sign wrong 
	XMVECTOR boxNormalV = calcNormal(faceIndex);
	XMStoreFloat3(&boxNormal, boxNormalV);
	//
	XMVECTOR nv = f.normalV;
	//XMStoreFloat3(&boxNormal, nv);
	replaceFace(faceId, boxNormal, newBoxHeight, scale, false, false);
	uint64_t capId = addCap(boxes[boxes.size() - 1]);
	parent.capId = capId;
	Box newBox = boxes[boxes.size() - 1];
	std::vector<int> newBranchIds;
	for (int s = 0; s < sides.size(); s++)
	{
		Branch newBranch;
		int face = getFaceIndexByID(newBox.faceIds[sides[s]]);
		XMFLOAT3 newNormal = rotateVector(boxNormalV, -calcNormal(face), angles[s]);
		replaceFace(face, newNormal, newBoxHeight * 6, scale, false, false);
		uint64_t newCapId = addCap(boxes[boxes.size() - 1]);
		newBranch.axis = newNormal;
		newBranch.parentAxis = parent.axis;
		newBranch.capId = newCapId;
		branchCaps.push_back(newCapId);
		parent.branches.push_back(newBranch);
	}
	// dont forget to store indices of caps to make more branches
	return parent.id;
}

// overload for normal issues, branch length, returns the branchCap ids
std::vector<uint64_t> QuadSystem::branch(Branch& parent, 
										int faceId, 
										std::vector<int> sides, 
										std::vector<float> angles, 
										float boxHeightRatio, 
										float branchLengthRatio, 
										bool polarity)
{
	int faceIndex = getFaceIndexByID(faceId);
	Face& f = faces[faceIndex];
	float newBoxHeight = approxWidth(faces[faceIndex]) * 0.8 * boxHeightRatio;
	XMFLOAT3 scale = XMFLOAT3(0.6, 0.6, 0.6);
	XMFLOAT3 boxNormal;
	// not sure why calcNormal gets the sign wrong 
	XMVECTOR boxNormalV = calcNormal(faceIndex);
	XMStoreFloat3(&boxNormal, boxNormalV);
	//
	XMVECTOR nv = f.normalV;
	//XMStoreFloat3(&boxNormal, nv);
	replaceFace(faceId, boxNormal, newBoxHeight, scale, polarity, false, false);
	uint64_t capId = addCap(boxes[boxes.size() - 1]);
	parent.capId = capId;
	Box newBox = boxes[boxes.size() - 1];
	std::vector<int> newBranchIds;
	std::vector<uint64_t> branchCapIds;

	for (int s = 0; s < sides.size(); s++)
	{
		Branch newBranch;
		int face = getFaceIndexByID(newBox.faceIds[sides[s]]);
		XMFLOAT3 newNormal = rotateVector(boxNormalV, -calcNormal(face), angles[s]);
		replaceFace(face, newNormal, newBoxHeight * 6, scale, polarity, false, false);
		uint64_t newCapId = addCap(boxes[boxes.size() - 1], polarity);
		newBranch.axis = newNormal;
		newBranch.parentAxis = parent.axis;
		newBranch.capId = newCapId;
		branchCaps.push_back(newCapId);
		parent.branches.push_back(newBranch);
		branchCapIds.push_back(newCapId);
	}
	// dont forget to store indices of caps to make more branches

	return branchCapIds;	
}

std::array<XMVECTOR, 4> QuadSystem::projectQuad(
	FXMVECTOR P1, FXMVECTOR P2, FXMVECTOR P3, FXMVECTOR P4,
	FXMVECTOR V, float L)
{
	XMVECTOR C = XMVectorScale(XMVectorAdd(XMVectorAdd(P1, P2), XMVectorAdd(P3, P4)), 0.25f);
	XMVECTOR N = XMVector3Normalize(V);

	// C2 = C + (L * N)
	XMVECTOR lenV = XMVectorReplicate(L);
	XMVECTOR V_offset = XMVectorMultiply(lenV, N); // V_offset = L * N
	XMVECTOR C2 = XMVectorAdd(C, V_offset);

	std::array<XMVECTOR, 4> originalPoints = { P1, P2, P3, P4 };
	std::array<XMVECTOR, 4> projectedPoints;

	for (int i = 0; i < originalPoints.size(); i++)
	{
		XMVECTOR P = originalPoints[i];
		XMVECTOR vec_PC2 = XMVectorSubtract(P, C2);
		XMVECTOR dotProductVec = XMVector3Dot(vec_PC2, N);
		XMVECTOR displacement = XMVectorMultiply(dotProductVec, N);
		XMVECTOR P_new = XMVectorSubtract(P, displacement);

		projectedPoints[i] = P_new;
	}

	return projectedPoints;
}

std::array<XMVECTOR, 4> QuadSystem::projectQuad(std::array<XMVECTOR, 4> originalPoints, FXMVECTOR V, float L)
{
	XMVECTOR C = findCentroidV(originalPoints);
	XMVECTOR N = XMVector3Normalize(V);

	// C2 = C + (L * N)
	XMVECTOR lenV = XMVectorReplicate(L);
	XMVECTOR V_offset = XMVectorMultiply(lenV, N); // V_offset = L * N
	XMVECTOR C2 = XMVectorAdd(C, V_offset);

	std::array<XMVECTOR, 4> projectedPoints;

	for (int i = 0; i < originalPoints.size(); i++)
	{
		XMVECTOR P = originalPoints[i];
		XMVECTOR vec_PC2 = XMVectorSubtract(P, C2);
		XMVECTOR dotProductVec = XMVector3Dot(vec_PC2, N);
		XMVECTOR displacement = XMVectorMultiply(dotProductVec, N);
		XMVECTOR P_new = XMVectorSubtract(P, displacement);
		projectedPoints[i] = P_new;
	}

	return projectedPoints;
}

void QuadSystem::buildBall(uint64_t faceId, float scale, bool cap, bool isBranch)
{
	// Get the face index
	int faceIndex = getFaceIndexByID(faceId);
	Face& f = faces[faceIndex];
	float faceWidth = approxWidth(f);
	//XMFLOAT3 centroid = findCentroid(f);
	std::array<int, 4> nearCorners = { f.face[0], f.face[1], f.face[2], f.face[3] };
	faceDeletionIdList.push_back(faceId);

	XMVECTOR dirVec = calcNormal(faceIndex);
	//XMFLOAT3 direction = f.normal;
	XMFLOAT3 direction;
	XMStoreFloat3(&direction, dirVec);

	buildBox(nearCorners, direction, faceWidth * scale * 0.33f, XMFLOAT3(scale, scale, scale));
	Box newBox = getBox(boxCount() - 1);
	std::array<int, 4> newCorners = newBox.farCorners();
	buildBox(newCorners, direction, faceWidth * scale * 0.33f, XMFLOAT3(1.0, 1.0f, 1.0));
	Box newBox2 = getBox(boxCount() - 1);
	std::array<int, 4> newCorners2 = newBox2.farCorners();
	buildBox(newCorners2, direction, faceWidth * scale * 0.33f, XMFLOAT3(1.0f / scale, 1.0f/scale, 1.0f / scale));
	Box newBox3 = getBox(boxCount() - 1);
	if (cap)
	{
		uint64_t capId = addCap(newBox3);
		capIds.push_back(capId);
	}
	// Add the new box to the list of boxes
	//boxes.push_back(newBox);
}

void QuadSystem::buildCylinder(XMFLOAT3 centerBase, XMFLOAT3 normal, float length, float baseRadius, float taper, int hDivs, int rDivs)
{
	Cylinder cyl;
	cyl.sideCount = rDivs;
	std::vector<XMFLOAT3> nearCorners;
	std::vector<XMFLOAT3> farCorners;
	float hSlice = length / (hDivs + 1);    // 0 divs: divide by 1, not zero
	float dRadius = (baseRadius * taper) / (hDivs + 1);
	// hDiv = 1 indicates a cylinder divided in half, so 1 + 2 for the base and the top rings = 3 rings
	int ringCount = hDivs + 2;
	int vInd = 0;
	for (int i = 0; i < ringCount; i++)
	{
		float y = i * hSlice;
		float r = baseRadius - (i * dRadius);
		float dTheta = 2.0 * XM_PI / rDivs;
		std::vector<XMFLOAT3> slicePoints = {};
		for (int j = 1; j <= rDivs; j++)
		{
			
			float c = cos(j * dTheta);
			float s = sin(j * dTheta);

			XMFLOAT3 point = XMFLOAT3(r * c, y, r * s);
			//slicePoints.push_back(point);
			points.push_back(point);
			cyl.verts.push_back(pointCount() - 1);

			if (i == 0)
			{
				cyl.baseVerts.push_back(pointCount() - 1);
			}
			if (i == ringCount - 1)
			{
				cyl.topVerts.push_back(pointCount() - 1);
			}
		}
	}
	int tInd = 0;
	for (int i = 0; i < (hDivs + 1); i++)
	{
		std::vector<uint64_t> ringFaces;
		for (int j = 0; j < (rDivs - 1); j++)
		{

			Face newFace;
			newFace.face.push_back(cyl.verts[i * rDivs + j]);
			newFace.face.push_back(cyl.verts[(i + 1) * rDivs + j]);
			newFace.face.push_back(cyl.verts[(i + 1) * rDivs + (j + 1)]);
			newFace.face.push_back(cyl.verts[i * rDivs + (j + 1)]);

			cyl.faceIndices.push_back(faceCount());
			faces.push_back(newFace);
			trackNewFace(faces.back(), faceCount() - 1);
			cyl.faceIds.push_back(faces.back().id);
			ringFaces.push_back(faces.back().id);
			/*
			tris[tInd] = i * rDivs + j;  // 0

			tris[tInd] = (i + 1) * rDivs + j;  // 4

			tris[tInd] = (i + 1) * rDivs + (j + 1);  // 5

			tris[tInd] = i * rDivs + (j + 1);  // 1
			*/

		}
		
		Face newFace;
		newFace.face.push_back(cyl.verts[(i + 1) * rDivs - 1]);
		newFace.face.push_back(cyl.verts[(i + 2) * rDivs - 1]);
		newFace.face.push_back(cyl.verts[(i + 1) * rDivs]);
		newFace.face.push_back(cyl.verts[(i + 1) * rDivs - rDivs]);

		cyl.faceIndices.push_back(faceCount());
		faces.push_back(newFace);
		trackNewFace(faces.back(), faceCount() - 1);
		cyl.faceIds.push_back(faces.back().id);
		ringFaces.push_back(faces.back().id);

		cyl.slices.push_back(ringFaces);


		/*
		tris[tInd] = (i + 1) * rDivs - 1;  // 3

		tris[tInd] = (i + 2) * rDivs - 1;  // 7

		tris[tInd] = (i + 1) * rDivs;  // 4

		tris[tInd] = (i + 1) * rDivs - rDivs;  // 0
		*/
	}

	cyl.direction = normal;
	cylinders.push_back(cyl);
	//boxes.push_back(box);
}

uint64_t QuadSystem::topCylinderCap(Cylinder& cyl)
{
	int numVerts = cyl.sideCount;
	// Create a new face for the cap
	Face capFace;
	for (int i = 0; i < numVerts; i++)
	{
		capFace.face.push_back(cyl.topVerts[i]);
	}
	// Add the cap face to the list of faces
	faces.push_back(capFace);
	trackNewFace(faces.back(), faceCount() - 1);
	uint64_t capId = faces.back().id;
	cyl.topFaceId = capId;	
	return capId;
}

uint64_t QuadSystem::bottomCylinderCap(Cylinder& cyl)
{
	int numVerts = cyl.sideCount;
	// Create a new face for the cap
	Face capFace;
	for (int i = 0; i < numVerts; i++)
	{
		capFace.face.push_back(cyl.baseVerts[i]);
	}
	// Add the cap face to the list of faces
	faces.push_back(capFace);
	trackNewFace(faces.back(), faceCount() - 1);
	uint64_t capId = faces.back().id;
	cyl.topFaceId = capId;
	return capId;
}


void QuadSystem::buildPlane(int xCount, int zCount)
{
	int currentPointCount = points.size();
	int currentFaceCount = faceCount();
	std::unique_ptr<float[]> xAxis = std::make_unique<float[]>(xCount);
	std::unique_ptr<float[]> zAxis = std::make_unique<float[]>(zCount);
	float xLim = 12.0;
	float yLim = 12.0;
	float step = .1;


	for (int i = 0; i < xCount; i++)
	{
		xAxis[i] = i * step;
	}
	for (int i = 0; i < zCount; i++)
	{
		zAxis[i] = i * step;
	}
	double pi = 3.1415926535;
	int vInd = 0;
	for (int i = 0; i < xCount; i++)
	{
		for (int j = 0; j < zCount; j++)
		{
			XMFLOAT3 point;
			point.x = xAxis[i];
			point.y = 0.0f;
			point.z = zAxis[j];
			points.push_back(point);
			vInd++;
		}
	}
	/*
	int tInd = 0;
	for (int i = 0; i < xCount - 1; i++)
	{
		for (int j = 0; j < zCount - 1; j++)
		{
			tris[tInd] = (i * zCount) + j;
			tris[tInd] = (i * zCount) + j + zCount;
			tris[tInd] = (i * zCount) + j + zCount + 1;
			tris[tInd] = (i * zCount) + j + 1;
			
		}
	}


	*/
}

XMFLOAT3 QuadSystem::rotateVector(XMVECTOR A, XMVECTOR B, float angle)
{
	angle = XMConvertToRadians(angle);
	// Calculate the Plane Normal (Rotation Axis)
	XMVECTOR N = XMVector3Cross(A, B);

	/*
	// Check if the vectors are parallel/anti-parallel (Cross product is near zero)
	if (XMVector3NearEqual(N, XMVectorZero(), XMVectorReplicate(1e-6f)))
	{
		return A;
	}
	*/

	XMVECTOR unitNormal = XMVector3Normalize(N);

	// Create the Rotation Matrix (Quaternion for smooth rotation)
	// Create a rotation quaternion for the delta_angle around the N_unit axis.
	XMVECTOR rotationQuat = XMQuaternionRotationAxis(unitNormal, angle);

	// Rotate Vector A
	// Use XMVector3Rotate to apply the quaternion rotation to vector A.
	XMVECTOR rotatedB = XMVector3Rotate(B, rotationQuat);
	XMFLOAT3 rotatedBfloat3;
	XMStoreFloat3(&rotatedBfloat3, rotatedB); // Stores the XMVECTOR into the XMFLOAT3

	return rotatedBfloat3;
}

void QuadSystem::deleteStagedFaces()
{
	std::vector<int> indicesToDelete;
	for (uint64_t id : faceDeletionIdList)
	{
		// in case a face was already deleted or replaced multiple times.
		try {
			indicesToDelete.push_back(faceIdToIndexMap.at(id));
		}
		catch (...) {
			continue;
		}
	}

	// sort indices
	std::sort(indicesToDelete.rbegin(), indicesToDelete.rend());

	for (int index : indicesToDelete)
	{
		faces.erase(faces.begin() + index);
	}

	faceIdToIndexMap.clear();
	for (int i = 0; i < faces.size(); ++i) {
		// The face's ID has not changed, but its index has
		faceIdToIndexMap[faces[i].id] = i;
	}

	faceDeletionIdList.clear();
}

void QuadSystem::buildAHES(HalfEdgeSystem& heSystem)
{
	// Clear all AHES containers to start fresh
	heSystem.edges.clear();
	heSystem.vertices.clear();
	heSystem.faces.clear();

	// 1. Pre-allocate size and initialize
	heSystem.vertices.resize(points.size());
	heSystem.faces.resize(faces.size());
	heSystem.edges.reserve(faces.size() * 4); // Roughly 4 edges per face

	// Temporary map to find twin half-edges: maps Canonical Edge -> Index of the first HE found
	// The use of std::map<EdgePair, HEIndex> allows for fast twin lookup and is transient.
	std::map<EdgePair, HEIndex> edgeToHalfEdgeMap;

	for (size_t i = 0; i < points.size(); ++i) {
		heSystem.vertices[i].outgoingEdge = -1;
	}
	// No need to initialize face boundingEdge here, it's set in the loop below.

	// 2. Iterate through faces to create Half-Edges and find Twins
	for (size_t f_idx = 0; f_idx < faces.size(); ++f_idx) {
		const Face& currentFace = faces[f_idx];
		int N = currentFace.face.size(); // Number of vertices in the N-gon

		std::vector<HEIndex> currentHEIndices;
		currentHEIndices.reserve(N);

		for (int i = 0; i < N; ++i) {
			int v_start_idx = currentFace.face[i];
			int v_end_idx = currentFace.face[(i + 1) % N];

			// Create the new half-edge (HE)
			HalfEdge newHE;
			newHE.origin = v_start_idx;
			newHE.face = f_idx;
			newHE.twin = -1; // Uninitialized

			HEIndex he_idx = heSystem.edges.size();
			heSystem.edges.push_back(newHE);
			currentHEIndices.push_back(he_idx);

			// --- A) Setup Vertex and Face Pointers ---
			heSystem.vertices[v_start_idx].outgoingEdge = he_idx;
			heSystem.faces[f_idx].boundingEdge = he_idx;

			// --- B) Setup Next Pointer ---
			if (i > 0) {
				heSystem.edges[currentHEIndices[i - 1]].next = he_idx;
			}

			// --- C) Find Twin using the Temporary Map ---
			EdgePair canonicalEdge = makeCanonicalEdgePair(v_start_idx, v_end_idx);

			auto it = edgeToHalfEdgeMap.find(canonicalEdge);
			if (it != edgeToHalfEdgeMap.end()) {
				// Found the twin HE
				HEIndex twin_he_idx = it->second;

				// Link them both ways
				heSystem.edges[he_idx].twin = twin_he_idx;
				heSystem.edges[twin_he_idx].twin = he_idx;

				// Remove the twin from the map (edge is now complete)
				edgeToHalfEdgeMap.erase(it);

			}
			else {
				// First time seeing this edge. Store this HE index.
				edgeToHalfEdgeMap[canonicalEdge] = he_idx;
			}
		} // End of face edges loop

		// Finalize 'next' pointer for the last HE to wrap back to the first
		if (N > 0) {
			heSystem.edges[currentHEIndices.back()].next = currentHEIndices.front();
		}
	} // End of faces loop
}




// Catmull-Clark algorithms


// =======================================================================
// Catmull-Clark Subdivision using AHES (O(V+E+F))
// =======================================================================

void QuadSystem::CCsubdivideHE1(float paramA, float paramB, float paramC)
{
	// --- Phase 1: Build Auxiliary Half-Edge System (AHES) ---
	HalfEdgeSystem heMesh;
	buildAHES(heMesh);

	// Get sizes for index offsets
	const size_t numOriginalVerts = points.size();
	// Total unique edges is the number of half-edges divided by 2
	const size_t numOriginalEdges = heMesh.edges.size() / 2;
	const size_t numOriginalFaces = faces.size();

	// Define the index offsets for new points in the final list:
	const size_t offsetEP = numOriginalVerts;
	const size_t offsetFP = numOriginalVerts + numOriginalEdges;

	// --- Phase 2: Calculate New Geometry (FP, EP, VP') ---

	// 1. Calculate Face Points (FP) - O(F * N) (N is avg vertices per face)
	for (HFaceIndex f_idx = 0; f_idx < numOriginalFaces; ++f_idx) {
		HFace& hFace = heMesh.faces[f_idx];
		HEIndex startHEIndex = hFace.boundingEdge;

		std::vector<XMFLOAT3> faceVerts;

		HEIndex currentHE = startHEIndex;
		do {
			// Traverse HEs around the face to get the original vertex positions
			faceVerts.push_back(points[heMesh.edges[currentHE].origin]);
			currentHE = heMesh.edges[currentHE].next;
		} while (currentHE != startHEIndex);

		hFace.facePoint = findNgonCentroid(faceVerts);
	}

	// 2. Calculate Edge Points (EP) - O(E)
	// Map canonical edges to their index in the newPoints list
	std::map<EdgePair, int> canonicalEdgeToNewPointIndex;
	int ep_index_counter = 0;

	for (HEIndex he_idx = 0; he_idx < heMesh.edges.size(); ++he_idx) {
		HalfEdge& he = heMesh.edges[he_idx];

		// Only calculate for one half of the twin pair (he_idx < he.twin ensures each unique edge is processed once)
		if (he_idx > he.twin) {
			continue;
		}

		const HalfEdge& twinHE = heMesh.edges[he.twin];

		// P (Edge midpoint of original vertices)
		XMFLOAT3 P = edgeMidpoint(points[he.origin], points[twinHE.origin]);

		// F1, F2 (Face Points of adjacent faces)
		// Accessing these points via HE.face is O(1)
		XMFLOAT3 F1 = heMesh.faces[he.face].facePoint;
		XMFLOAT3 F2 = heMesh.faces[twinHE.face].facePoint;

		// R (Average of adjacent face points)
		XMFLOAT3 R = edgeMidpoint(F1, F2);

		// New Edge Point = 0.5 * (P + R)
		XMFLOAT3 newEdgePoint;
		XMVECTOR v_new_edge = XMVectorScale(XMVectorAdd(XMLoadFloat3(&P), XMLoadFloat3(&R)), 0.5f);
		DirectX::XMStoreFloat3(&newEdgePoint, v_new_edge);

		// Store the result on both HEs for easy lookup during mesh construction
		he.edgePoint = newEdgePoint;
		heMesh.edges[he.twin].edgePoint = newEdgePoint;

		// Map this edge point to its final index
		canonicalEdgeToNewPointIndex[makeCanonicalEdgePair(he.origin, heMesh.edges[he.twin].origin)] = offsetEP + ep_index_counter;
		ep_index_counter++;
	}


	// 3. Calculate Vertex Points (VP') - O(V * Valence)
	for (HVertexIndex v_idx = 0; v_idx < numOriginalVerts; ++v_idx) {
		HVertex& hVert = heMesh.vertices[v_idx];

		HEIndex startHEIndex = hVert.outgoingEdge;
		if (startHEIndex == -1) {
			hVert.vertexPoint = points[v_idx];
			continue;
		}

		std::vector<XMFLOAT3> adjFacePoints; // For F average
		std::vector<XMFLOAT3> adjEdgeMids;   // For R average (P in the CC formula)

		HEIndex currentHE_idx = startHEIndex;
		int n = 0; // Valence of the vertex

		// Traverse around the vertex using the O(1) twin->next cycle
		do {
			// 1. Explicitly create references to the HalfEdge structs using the index
			const HalfEdge& currentHE_ref = heMesh.edges[currentHE_idx]; // <<< Correct: use index to get struct reference
			const HalfEdge& twinHE_ref = heMesh.edges[currentHE_ref.twin];

			// F: Sum of adjacent Face Points
			adjFacePoints.push_back(heMesh.faces[currentHE_ref.face].facePoint);

			// R: Sum of adjacent edge midpoints. This line is now correct:
			adjEdgeMids.push_back(edgeMidpoint(points[currentHE_ref.origin], points[twinHE_ref.origin]));

			// Move to the next outgoing edge from V: he = he.twin.next
			currentHE_idx = twinHE_ref.next;
			n++;
		} while (currentHE_idx != startHEIndex && n < heMesh.edges.size());

		XMFLOAT3 F_avg = findNgonCentroid(adjFacePoints); // Average of FPs
		XMFLOAT3 R_avg = findNgonCentroid(adjEdgeMids);   // Average of midpoints R_i
		XMFLOAT3 P = points[v_idx];                       // Original vertex point P

		// Catmull-Clark formula for the new vertex point (VP')
		// VP' = ((n - A) * P + B * F_avg + C * R_avg) / n
		float n_float = static_cast<float>(n);

		float m1 = (n_float - paramA) / n_float;
		float m2 = paramB / n_float;
		float m3 = paramC / n_float;

		XMVECTOR v_new_vert = XMVectorAdd(
			XMVectorScale(XMLoadFloat3(&P), m1),
			XMVectorAdd(
				XMVectorScale(XMLoadFloat3(&F_avg), m2),
				XMVectorScale(XMLoadFloat3(&R_avg), m3)
			)
		);

		DirectX::XMStoreFloat3(&hVert.vertexPoint, v_new_vert);
	}


	// --- Phase 3: Construct the new mesh ---

	std::vector<XMFLOAT3> newPoints;
	std::vector<Face> newFaces;

	// 1. Add new points in order (VP', EP, FP)
	// VP' (New Vertex Points) - 0 to numOriginalVerts - 1
	for (const auto& hVert : heMesh.vertices) {
		newPoints.push_back(hVert.vertexPoint);
	}

	// EP (Edge Points) - offsetEP to offsetFP - 1 (Already added via canonicalEdgeToNewPointIndex map)
	// We add them by iterating through the map since the map guarantees unique edges and ordered indices
	// Note: The total size is known, so we resize and fill if preferred, but map iteration ensures correctness
	newPoints.resize(offsetFP + numOriginalFaces);
	for (const auto& pair : canonicalEdgeToNewPointIndex) {
		// Find the edge point stored on one of the corresponding half-edges
		const HalfEdge& he = heMesh.edges[heMesh.edges[heMesh.vertices[pair.first.v1].outgoingEdge].twin];
		newPoints[pair.second] = he.edgePoint;
	}


	// FP (Face Points) - offsetFP onwards
	int fp_idx_counter = offsetFP;
	for (const auto& hFace : heMesh.faces) {
		newPoints[fp_idx_counter++] = hFace.facePoint;
	}

	/*
	// 2. Construct new faces (Quads) - O(F * N)
	for (HFaceIndex f_idx = 0; f_idx < numOriginalFaces; ++f_idx) {
		const HFace& hFace = heMesh.faces[f_idx];
		HEIndex startHEIndex = hFace.boundingEdge;

		HEIndex currentHE = startHEIndex;

		// Traverse HEs around the original face to create N quads
		do {
			const HalfEdge& he = heMesh.edges[currentHE];
			// 1. Get the HalfEdge whose 'next' is 'he'. This is the HE leaving V_curr *on the adjacent face*. We need its index.
			HEIndex prev_HE_candidate_idx = heMesh.edges[he.twin].next;

			// 2. The twin of that candidate is the HE running V_prev -> V_curr. This is the index we want.
			HEIndex prev_HE_idx = heMesh.edges[prev_HE_candidate_idx].twin;

			// 3. Get the final HalfEdge reference using the correct index.
			const HalfEdge& prevHE = heMesh.edges[prev_HE_idx];
			//const HalfEdge& prevHE = heMesh.edges[heMesh.edges[he.twin].next].twin;
			// ^ The HE whose origin is the previous vertex (V_prev -> V_curr)

			Face newQuad;

			// P1: V' (New Vertex Point) - Index is the same as the old vertex index
			newQuad.face.push_back(he.origin);

			// P2: E'_{current} (Edge Point of the current edge)
			EdgePair currentEdge = makeCanonicalEdgePair(he.origin, heMesh.edges[he.twin].origin);
			newQuad.face.push_back(canonicalEdgeToNewPointIndex.at(currentEdge));

			// P3: F' (Face Point) - Index is offsetFP + original_face_index
			newQuad.face.push_back(offsetFP + f_idx);

			// P4: E'_{prev} (Edge Point of the previous edge)
			EdgePair prevEdge = makeCanonicalEdgePair(prevHE.origin, he.origin);
			newQuad.face.push_back(canonicalEdgeToNewPointIndex.at(prevEdge));

			newFaces.push_back(newQuad);

			currentHE = he.next;
		} while (currentHE != startHEIndex);
	}
	*/

	// 2. Construct new faces (Quads) - O(F * N)
	for (HFaceIndex f_idx = 0; f_idx < numOriginalFaces; ++f_idx) {
		const HFace& hFace = heMesh.faces[f_idx];
		HEIndex startHEIndex = hFace.boundingEdge;
		HEIndex currentHE_idx = startHEIndex;

		// For every vertex in the original face, we create one new quad
		do {
			const HalfEdge& he = heMesh.edges[currentHE_idx];

			// To find the "previous" edge point relative to the current vertex 'he.origin':
			// We need the edge that ends at he.origin. In AHES, this is the 'next' 
			// of the Half-Edge that comes before 'he' in the face loop.
			// However, a faster way in AHES to find the "incoming" edge to he.origin 
			// inside this face is to find the HE whose 'next' is currentHE_idx.

			HEIndex prev_in_face_idx = -1;
			HEIndex search_idx = startHEIndex;
			do {
				if (heMesh.edges[search_idx].next == currentHE_idx) {
					prev_in_face_idx = search_idx;
					break;
				}
				search_idx = heMesh.edges[search_idx].next;
			} while (search_idx != startHEIndex);

			const HalfEdge& prev_he = heMesh.edges[prev_in_face_idx];

			Face newQuad;
			/* WINDING ORDER (Counter-Clockwise):
			   1. The New Vertex Position (VP')
			   2. The Edge Point of the edge STARTING at this vertex (EP_curr)
			   3. The Face Point (FP)
			   4. The Edge Point of the edge ENDING at this vertex (EP_prev)
			*/

			// 1. VP'
			newQuad.face.push_back(he.origin);

			// 2. EP_curr (Edge: he.origin -> next_vert)
			EdgePair currEdge = makeCanonicalEdgePair(he.origin, heMesh.edges[he.twin].origin);
			newQuad.face.push_back(canonicalEdgeToNewPointIndex.at(currEdge));

			// 3. FP'
			newQuad.face.push_back(offsetFP + f_idx);

			// 4. EP_prev (Edge: prev_vert -> he.origin)
			EdgePair prevEdge = makeCanonicalEdgePair(prev_he.origin, he.origin);
			newQuad.face.push_back(canonicalEdgeToNewPointIndex.at(prevEdge));

			newFaces.push_back(newQuad);

			currentHE_idx = he.next;
		} while (currentHE_idx != startHEIndex);
	}

	// --- Phase 4: Replace old mesh data ---
	points = std::move(newPoints);
	faces = std::move(newFaces);
}




void QuadSystem::CCsubdivideNgon(float paramA = 3.0, float paramB = 2.0, float paramC = 1.0)
{
	// --- Phase 1: Build Adjacency Lists (Mostly unchanged, robust for N-gons) ---
	findEdges();

	std::unordered_map<int, std::vector<int>> pointToFacesMap;
	std::unordered_map<int, std::array<int, 2>> edgeToFacesMap;
	std::unordered_map<int, std::vector<int>> faceToEdgesMap;
	std::unordered_map<int, std::vector<int>> pointToEdgesMap;

	// point to faces
	for (int f_idx = 0; f_idx < faces.size(); ++f_idx) {
		const Face& face = faces[f_idx];
		for (int p_index : face.face) {
			pointToFacesMap[p_index].push_back(f_idx);
		}
	}

	// edge to faces
	for (int e_idx = 0; e_idx < edges.size(); ++e_idx) {
		const auto& edge = edges[e_idx];
		std::array<int, 2> adjFaces = { -1, -1 };
		int faceCount = 0;

		for (int f_idx = 0; f_idx < faceEdgePairs.size(); ++f_idx) {
			if (std::find(faceEdgePairs[f_idx].begin(), faceEdgePairs[f_idx].end(), edge) != faceEdgePairs[f_idx].end()) {
				if (faceCount < 2) {
					adjFaces[faceCount] = f_idx;
					faceCount++;
				}
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
			auto it = std::find(edges.begin(), edges.end(), faceEdge);
			if (it != edges.end()) {
				int edge_idx = std::distance(edges.begin(), it);
				adjEdges.push_back(edge_idx);
			}
		}
		faceToEdgesMap[f] = adjEdges;
	}

	// point to edges
	for (int v = 0; v < points.size(); ++v) {
		std::vector<int> adjEdges;
		for (int e = 0; e < edges.size(); ++e) {
			const auto& edge = edges[e];
			if (edge.first == v || edge.second == v) {
				adjEdges.push_back(e);
			}
		}
		pointToEdgesMap[v] = adjEdges;
	}

	// --- Phase 2: Calculate New Geometry (Robust for N-gons) ---

	std::unordered_map<int, XMFLOAT3> facePoints;
	std::unordered_map<int, XMFLOAT3> edgePoints;
	std::unordered_map<int, XMFLOAT3> vertPoints;

	// 1. calculate face points (Centroid of N-gon)
	for (int f_idx = 0; f_idx < faces.size(); ++f_idx) {
		std::vector<XMFLOAT3> faceVerts;
		for (int v_idx : faces[f_idx].face) {
			faceVerts.push_back(points[v_idx]);
		}
		facePoints[f_idx] = findNgonCentroid(faceVerts);
	}

	// 2. calculate edge points (Average of Midpoint and Face Averages)
	for (int e = 0; e < edges.size(); ++e) {
		const auto& edge = edges[e];

		// Handle boundary edges (only 1 adjacent face) by using the face point twice 
		// or by using 0.0 for the missing face (simplest implementation)

		// Edge midpoint (P in the algorithm)
		XMFLOAT3 edgeMid = edgeMidpoint(points[edge.first], points[edge.second]);

		// Average of adjacent face points (R in the algorithm)
		XMFLOAT3 fp1 = facePoints[edgeToFacesMap[e][0]];
		XMFLOAT3 fp2 = facePoints[edgeToFacesMap[e][1]];
		XMFLOAT3 avgFP = edgeMidpoint(fp1, fp2); // Simple average of 2 points

		// New edge point is the average of P and R
		XMFLOAT3 newEdgePoint;
		XMVECTOR v_new_edge = XMVectorScale(XMVectorAdd(XMLoadFloat3(&edgeMid), XMLoadFloat3(&avgFP)), 0.5f);
		DirectX::XMStoreFloat3(&newEdgePoint, v_new_edge);

		edgePoints[e] = newEdgePoint;
	}

	// 3. calculate vertex points (Smoothing Formula)
	for (int v = 0; v < points.size(); ++v) {
		// Find average of adjacent face points (F)
		std::vector<XMFLOAT3> adjFacePoints;
		const auto& adjFaces = pointToFacesMap[v];
		for (int f_idx : adjFaces) {
			adjFacePoints.push_back(facePoints[f_idx]);
		}
		XMFLOAT3 F = findNgonCentroid(adjFacePoints); // R in the paper

		// Find average of adjacent edge midpoints (R)
		std::vector<XMFLOAT3> adjEdgeMids;
		for (int e_idx : pointToEdgesMap[v]) {
			const auto& edge = edges[e_idx];
			adjEdgeMids.push_back(edgeMidpoint(points[edge.first], points[edge.second]));
		}
		XMFLOAT3 R = findNgonCentroid(adjEdgeMids); // Q in the paper

		// Original vertex point (P)
		XMFLOAT3 P = points[v];

		int n = adjFaces.size(); // number of adjacent faces/edges

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
		vertPoints[v] = newVertPoint;
	}

	// --- Phase 3: Construct the new mesh ---
	std::vector<XMFLOAT3> newPoints;
	std::vector<Face> newFaces;

	// Add new vertex points (n1 total)
	for (int i = 0; i < points.size(); ++i) {
		newPoints.push_back(vertPoints[i]);
	}
	// Add new edge points (n2 total)
	for (int i = 0; i < edges.size(); ++i) {
		newPoints.push_back(edgePoints[i]);
	}
	// Add new face points (n3 total)
	for (int i = 0; i < faces.size(); ++i) {
		newPoints.push_back(facePoints[i]);
	}

	// Index offsets for the new points
	size_t n1 = points.size(); // Start index for Edge Points
	size_t n2 = edges.size();  // Total Edge Points
	size_t n3_start = n1 + n2; // Start index for Face Points (n1 + n2)

	// Construct new faces (Quads) - Splits an N-gon into N Quads
	for (int f = 0; f < faces.size(); ++f) {
		const auto& oldFace = faces[f];
		int N = oldFace.face.size(); // Number of vertices in the old face

		// The Face Point index is constant for all N quads derived from this face
		int FP_idx = n3_start + f;

		for (int i = 0; i < N; ++i) {
			Face newQuad;

			// 1. V': New Vertex Point (Original index)
			newQuad.face.push_back(oldFace.face[i]);

			// 2. E'_{i}: Edge Point of the current/next edge (index i in the faceToEdgesMap)
			// Edge index in the edges list is: faceToEdgesMap[f][i]
			newQuad.face.push_back(n1 + faceToEdgesMap[f][i]);

			// 3. F': Face Point (Constant for all quads in this face)
			newQuad.face.push_back(FP_idx);

			// 4. E'_{i-1}: Edge Point of the previous edge (index i-1 in the faceToEdgesMap)
			// THIS IS THE CRITICAL FIX for N-gons:
			int prev_i = (i == 0) ? N - 1 : i - 1; // Correct way to get the previous index
			// Using the robust modulo: int prev_i = (i - 1 + N) % N; // Also works
			newQuad.face.push_back(n1 + faceToEdgesMap[f][prev_i]);

			newFaces.push_back(newQuad);
		}
	}

	// Replace the old mesh with the new one
	points = newPoints;
	faces = newFaces;
}


// Quads only
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
	// faster, hash lookup, no order
	for (int f = 0; f < faces.size(); ++f) {
		const Face& face = faces[f];
		for (int p : face.face) {
			pointToFacesMap[p].push_back(f);
		}
	}

	// edge to faces
	for (int e = 0; e < edges.size(); ++e) {
		const auto& edge = edges[e];

		// Replicates the Python 'adjFaces = []' logic
		std::array<int, 2> adjFaces = { -1, -1 };
		int faceCount = 0;

		// Replicates the Python 'for i in range(len(eList))' and 'if edges[e] in eList[i]'
		for (int f = 0; f < faceEdgePairs.size(); ++f) {
			// std::find is the C++ equivalent of Python's 'in' for a list
			if (std::find(faceEdgePairs[f].begin(), faceEdgePairs[f].end(), edge) != faceEdgePairs[f].end()) {
				if (faceCount < 2) {
					adjFaces[faceCount] = f;
					faceCount++;
				}
				// We can stop searching for this edge if we've found both faces
				if (faceCount == 2) {
					break;
				}
			}
		}
		edgeToFacesMap[e] = adjFaces;
	}

	// face to edges
	for (int f = 0; f < faceEdgePairs.size(); ++f) {
		std::vector<int> adjEdges;
		for (const auto& faceEdge : faceEdgePairs[f]) {
			// This is the slow part: finding the index of an edge in the uniqueEdges list
			// This is needed because the map key is an int index.
			auto it = std::find(edges.begin(), edges.end(), faceEdge);
			if (it != edges.end()) {
				int e = std::distance(edges.begin(), it);
				adjEdges.push_back(e);
			}
		}
		faceToEdgesMap[f] = adjEdges;
	}

	// point to edges
	// Loop through each vertex in the mesh
	for (int v = 0; v < points.size(); ++v) {
		std::vector<int> adjEdges;
		// brute-force search
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

	// calculate face points (centroids)
	facePoints.clear();
	for (int f = 0; f < faces.size(); ++f) {
		std::vector<XMFLOAT3> faceVerts;
		for (int v : faces[f].face) {
			faceVerts.push_back(points[v]);
		}
		facePoints[f] = findNgonCentroid(faceVerts);
	}

	// calculate edge points
	edgePoints.clear();
	for (int e = 0; e < edges.size(); ++e) {
		// std::vector<std::pair<int, int>>
		const auto& edge = edges[e];

		// Edge midpoint (P in the algorithm)
		XMFLOAT3 edgeMid = edgeMidpoint(points[edge.first], points[edge.second]);

		// Average of adjacent face points (R in the algorithm)
		XMFLOAT3 fp1 = facePoints[edgeToFacesMap[e][0]];
		XMFLOAT3 fp2 = facePoints[edgeToFacesMap[e][1]];
		XMFLOAT3 avgFP = edgeMidpoint(fp1, fp2);

		// New edge point is the average of P and R
		XMFLOAT3 newEdgePoint;
		XMVECTOR v_new_edge = XMVectorScale(XMVectorAdd(XMLoadFloat3(&edgeMid), XMLoadFloat3(&avgFP)), 0.5f);
		DirectX::XMStoreFloat3(&newEdgePoint, v_new_edge);

		edgePoints[e] = newEdgePoint;
	}

	// calculate vertex points
	// find avg's of facePoints and edgePoints
	// incorporate Catmull-Clark weights
	vertPoints.clear();
	for (int v = 0; v < points.size(); ++v) {
		// Find average of adjacent face points (F)
		std::vector<XMFLOAT3> adjFacePoints;
		const auto& adjFaces = pointToFacesMap[v];
		for (int f : adjFaces) {
			adjFacePoints.push_back(facePoints[f]);
		}
		XMFLOAT3 F = findNgonCentroid(adjFacePoints);

		// Find average of adjacent edge midpoints (R)
		std::vector<XMFLOAT3> adjEdgeMids;
		for (int e : pointToEdgesMap[v]) {
			const auto& edge = edges[e];
			adjEdgeMids.push_back(edgeMidpoint(points[edge.first], points[edge.second]));
		}
		XMFLOAT3 R = findNgonCentroid(adjEdgeMids);

		// Original vertex point (P)
		XMFLOAT3 P = points[v];

		// n is the number of faces/edges adjacent to the vertex
		// get n from the new map's size
		int n = adjFaces.size();

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
		vertPoints[v] = newVertPoint;
	}

	//phase 3: subdivide and rebuild mesh

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

	// construct new faces (Quads)
	size_t n1 = points.size();
	size_t n2 = edges.size();

	for (int f = 0; f < faces.size(); ++f) {
		if (true)
		{
			const auto& oldFace = faces[f];
			const auto& oldEdges = faceToEdgesMap[f];

			// A temporary vector to hold the new faces for this old face
			std::vector<Face> subFaces;

			for (int i = 0; i < oldFace.face.size(); ++i) {
				Face newFace;
				// Original vertex point (V')
				newFace.face.push_back(oldFace.face[i]);
				// Edge point of the next edge (E')
				newFace.face.push_back(n1 + faceToEdgesMap[f][i]);
				// Face point (F')
				newFace.face.push_back(n1 + n2 + f);
				// Edge point of the previous edge (E'')
				newFace.face.push_back(n1 + faceToEdgesMap[f][(i + 3) % oldFace.face.size()]);

				subFaces.push_back(newFace);
			}
			newFaces.insert(newFaces.end(), subFaces.begin(), subFaces.end());
		}

	}

	// Replace the old mesh with the new one
	points = newPoints;
	faces = newFaces;

}

/*

// original

void QuadSystem::buildAHES(HalfEdgeSystem& heSystem)
{
	// Clear all AHES containers to start fresh
	heSystem.edges.clear();
	heSystem.vertices.clear();
	heSystem.faces.clear();

	// 1. Pre-allocate size (optimistic)
	heSystem.vertices.resize(points.size());
	heSystem.faces.resize(faces.size());
	heSystem.edges.reserve(faces.size() * 4); // Catmull-Clark guarantees quads after 1st pass

	// Temporary map to find twin half-edges: maps Canonical Edge -> Index of the first HE found
	// The key is the sorted pair (v_min, v_max). The value is the index of the HE (v_min -> v_max).
	std::map<EdgePair, HEIndex> edgeToHalfEdgeMap;

	// 2. Initialize Vertices and Faces
	for (size_t i = 0; i < points.size(); ++i) {
		heSystem.vertices[i].outgoingEdge = -1; // -1 = uninitialized
	}
	for (size_t i = 0; i < faces.size(); ++i) {
		heSystem.faces[i].originalFaceIndex = i;
		heSystem.faces[i].boundingEdge = -1;
	}

	// 3. Iterate through faces to create Half-Edges and find Twins
	for (size_t f_idx = 0; f_idx < faces.size(); ++f_idx) {
		const Face& currentFace = faces[f_idx];
		int N = currentFace.face.size(); // Number of vertices in the N-gon

		// Store the indices of the half-edges being created for this face
		std::vector<HEIndex> currentHEIndices;

		for (int i = 0; i < N; ++i) {
			int v_start_idx = currentFace.face[i];
			int v_end_idx = currentFace.face[(i + 1) % N];

			// Create the new half-edge (HE) for the path v_start -> v_end
			HalfEdge newHE;
			newHE.origin = v_start_idx;
			newHE.face = f_idx;
			newHE.twin = -1; // Will be set later

			HEIndex he_idx = heSystem.edges.size();
			heSystem.edges.push_back(newHE);
			currentHEIndices.push_back(he_idx);

			// --- A) Setup Vertex and Face Pointers ---
			// Set the outgoing edge for the vertex (just pick the last one created for simplicity)
			heSystem.vertices[v_start_idx].outgoingEdge = he_idx;
			// Set the bounding edge for the face
			heSystem.faces[f_idx].boundingEdge = he_idx;

			// --- B) Setup Next Pointer ---
			// The HE 'next' is the one created for the next face edge
			if (i > 0) {
				heSystem.edges[currentHEIndices[i - 1]].next = he_idx;
			}

			// --- C) Find Twin using the Temporary Map ---
			EdgePair canonicalEdge;
			canonicalEdge.v1 = std::min(v_start_idx, v_end_idx);
			canonicalEdge.v2 = std::max(v_start_idx, v_end_idx);

			// The edge key for the half-edge going in the opposite direction (v_end -> v_start)
			EdgePair twinKey = { v_end_idx, v_start_idx };

			// Search the map for the twin (canonical edge)
			auto it = edgeToHalfEdgeMap.find(canonicalEdge);
			if (it != edgeToHalfEdgeMap.end()) {
				// We found the twin (the HE that runs v_start -> v_end)
				HEIndex twin_he_idx = it->second;

				// Link them: Current HE links to Twin HE
				heSystem.edges[he_idx].twin = twin_he_idx;
				// Twin HE links back to Current HE
				heSystem.edges[twin_he_idx].twin = he_idx;

				// Remove the twin from the map to mark the edge as "closed"
				edgeToHalfEdgeMap.erase(it);

			}
			else {
				// First time seeing this edge (v_start -> v_end). Store this HE index.
				edgeToHalfEdgeMap[canonicalEdge] = he_idx;
			}
		} // End of face edges loop

		// Finalize 'next' pointer for the last HE to wrap back to the first
		if (N > 0) {
			heSystem.edges[currentHEIndices.back()].next = currentHEIndices.front();
		}
	} // End of faces loop

	// Cleanup: edgeToHalfEdgeMap is local and destroyed automatically.
}
*/

