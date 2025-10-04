#pragma once

#include <algorithm>
#include <unordered_map>
#include <cmath>
#include <vector>
#include <array>
#include <tuple>
#include <set>    
#include <optional>
#include <utility>
#include <DirectXMath.h>
#include "Vertex.h"
#include "Mesh.h"


// represents a flat quad, eventually subdivided and/or triangulated, tri's inherit normal, if calculated
// position data alone is sufficient for further operations, like subQuad and extrusion
// normal plus a center point yield an axis for extrusion and cylindrical coords
// use for redundant indexing (real flat shading) (not needed)
struct Quad
{
	XMFLOAT3 v1;
	XMFLOAT3 v2;
	XMFLOAT3 v3;
	XMFLOAT3 v4;

	XMFLOAT3 normal;
	XMVECTOR normalV;
	XMFLOAT3 centroid;
};

// traditional list of indices into the array of 3d coordinates
// polygons of any degree (everything should quads or tris eventually)
// CC subD only works with 4-gons, Loop subD for tris
struct Face
{
	std::vector<int> face;
	uint64_t id;
	XMVECTOR normalV;
	XMFLOAT3 normal;
	XMFLOAT3 centroid;
};

// the main structural component
struct Box
{
	uint64_t id;
	std::vector<int> verts;
	std::vector<Face> faces;
	std::vector<uint64_t> faceIds;
	std::vector<int> faceIndices;
	XMFLOAT3 direction;
	XMFLOAT3 scale = XMFLOAT3(1.0, 1.0, 1.0);

	// track the corners
	std::array<int, 4> nearCorners()
	{
		std::array<int, 4> corners = { verts[0], verts[1], verts[2], verts[3] };
		return corners;
	}

	std::array<int, 4> farCorners()
	{
		std::array<int, 4> corners = { verts[4], verts[5], verts[6], verts[7] };
		return corners;
	}
	
	
};

struct Branch
{
	std::vector<Branch> branches;
	uint64_t capId;
	int boxIndex;
};

// Custom Hash Function for std::pair<int, int>
struct pair_hash {
	template <class T1, class T2>
	std::size_t operator() (const std::pair<T1, T2>& pair) const {
		return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
	}
};


// represents a 3D mesh of quads, calculates normals and performs triangulation and/or subdivision
// provides functionality for procedural and randomized shape generation
// no access to GPU, only describes quad position data and converts to Mesh tri's 
// local coordinates at origin always, no internal position/rotation data
class QuadSystem
{
public:
	QuadSystem() {};

	// operations may need to be outsourced to separate subdivision classes
	std::vector<int> triangulateQuads();
	std::vector<int> triangulateFaces();
	XMVECTOR calcNormal(int p1, int p2, int p3);
	XMVECTOR calcNormal(int faceIndex);
	std::tuple<float, float> XYZtoUV(XMFLOAT3);
	XMFLOAT3 UVtoXYZ(Face face, std::tuple<float, float> uvCoord);
	XMFLOAT3 findCentroid(Face face); 
	XMFLOAT3 findCentroid(const std::vector<XMFLOAT3>& points);
	XMFLOAT3 edgeMidpoint(const XMFLOAT3& p1, const XMFLOAT3& p2);
	float distance(XMFLOAT3 pt1, XMFLOAT3 pt2);
	float approxWidth(Face f);
	std::vector<XMFLOAT3> scalePolygon(std::vector<XMFLOAT3> points, const XMFLOAT3 scaleOrigin, XMFLOAT3 scale);

	// tracking faces with unique ID's for deletions and looking up branch caps
	void trackNewFace(Face& f, int currentIndex) {
		// Assign unique ID and increment the counter
		f.id = nextFaceID++; 
		// Map the ID to its current location
		faceIdToIndexMap[f.id] = currentIndex; 
	}

	int getFaceIndexByID(uint64_t faceId) const {
		// Use .at() throws an exception if the ID doesn't exist.
		return faceIdToIndexMap.at(faceId);
	}

	

	// standard indexing
	std::unique_ptr<Mesh> convertQuadsToMesh();
	// redundant indexing
	std::unique_ptr<Mesh> convertFacesToMesh();

	// functions to define 3D structural components and random/procedural systems
	void buildFlatSquare(float length);

	void addPoints(std::vector<XMFLOAT3>);

	// main component of structures
	// always four sided 
	void buildBox(XMFLOAT3 start, XMFLOAT3 normal, float radius1, float radius2, float length);

	void buildBox(std::array<int, 4> nearCorners, std::array<int, 4> farCorners);

	void buildBox(std::array<int, 4> nearCorners, XMFLOAT3 normal, float length, XMFLOAT3 endScale);

	// replace a face with a box
	//void replaceFace(int faceIndex, XMFLOAT3 normal, float length, XMFLOAT3 endScale, bool cap);
	void replaceFace(uint64_t faceId, XMFLOAT3 normal, float length, XMFLOAT3 endScale, bool cap);

	// caps
	void topCap(Box& box);
	void bottomCap(Box& box);

	// connections
	void connectBoxes(Box box1, Box box2);

	// given a point in space and a normal vector, returns a list of 4 orthogonal unit vectors
	std::array<XMFLOAT3, 4> orthoUnits(XMFLOAT3 location, XMVECTOR normal);

	// deconstructs boxes to fill points, lines and faces lists
	void fillLists();

	// subdivision alorithms
	void loopSubdivide();
	void CCsubdivide(float paramA, float paramB, float paramC);

	// two perpendicular cuts, one face becomes 4
	void divideFace(int faceIndex, float dim1, float dim2, float u, float v);
	// parallel cuts
	void sliceFace(int faceIndex, float edge1, int numSections);

	void branch(int faceIndex, std::vector<int> sides, float split, std::vector<float> angles, float widthPercent, float boxHeightRatio);

	void branchSystem();

	// A and B two  XMVECTORs
	XMFLOAT3 rotateVector(XMVECTOR A, XMVECTOR B, float angle)
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

	void findEdges();

	int pointCount()
	{
		return points.size();
	}

	int faceCount()
	{
		return faces.size();
	}

	int boxCount()
	{
		return boxes.size();
	}

	Box& getBox(int index)
	{
		return boxes[index];
	}

	bool faceExists(int index)
	{
		//return faces[index].face[0] != -1;
		return true;
	}

	bool faceExists(const Face f)
	{
		//return f.face[0] != -1;
		return true;
	}

	std::vector<Face>& getFaces()
	{
		return faces;
	}

	void deleteFace(int index)
	{
		faces.erase(faces.begin() + index);
	}

	
	void deleteStagedFaces()
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

private:
	uint64_t nextFaceID = 0;
	std::vector<XMFLOAT3> points;
	std::vector<Face> faces;
	std::vector<std::pair<int, int>> edges;
	std::vector<std::vector<std::pair<int, int>>> faceEdgePairs;
	std::vector<Quad> quads;
	std::vector<Box> boxes;
	std::unordered_map<uint64_t, int> faceIdToIndexMap;
	std::vector<uint64_t> faceDeletionIdList;
	std::vector<int> branchCaps;

	// standard indexing for quad->triangle conversion
	std::array<int, 6> triIndices = { 0, 1, 3, 3, 1, 2 };

	XMFLOAT3 origin = XMFLOAT3(0.0f, 0.0f, 0.0f);
};
