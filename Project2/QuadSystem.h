#pragma once
#define NOMINMAX

#include <algorithm>
#include <unordered_map>
#include <map>
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
	//std::vector<Face> faces;
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

struct Cylinder
{
	uint64_t id;
	uint64_t baseFaceId;
	uint64_t topFaceId;
	int sliceCount;
	int sideCount;
	std::vector<int> verts;
	std::vector<uint64_t> faceIds;
	std::vector<int> faceIndices;
	XMFLOAT3 direction;
	XMFLOAT3 scale = XMFLOAT3(1.0, 1.0, 1.0);

	// track the top and bottom vertices
	std::vector<int> baseVerts;
	std::vector<int> topVerts;
	
	// track the slices by face ids
	std::vector<std::vector<uint64_t>> slices;


};

struct Branch
{
	uint16_t level = 0;
	int id = 0;
	std::vector<int> boxes;
	std::vector<Branch> branches;
	uint64_t capId;
	XMFLOAT3 axis;
	XMFLOAT3 parentAxis;
};

// Custom Hash Function for std::pair<int, int>
struct pair_hash {
	template <class T1, class T2>
	std::size_t operator() (const std::pair<T1, T2>& pair) const {
		return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
	}
};

// Use int indices into the corresponding vector of HEDS elements.
using HEIndex = int;
using HVertexIndex = int;
using HFaceIndex = int;

// 1. HalfEdge
struct HalfEdge {
	HEIndex next;       // Next HE in the face loop (O(1) Face Adjacency)
	HEIndex twin;       // Opposite HE on the adjacent face (O(1) Edge Adjacency)
	HVertexIndex origin; // Origin vertex index
	HFaceIndex face;     // Incident face index

	// Custom data needed for Catmull-Clark
	XMFLOAT3 edgePoint; // The newly calculated Edge Point (EP)
};

// 2. HVertex (References the Original points for coordinates)
struct HVertex {
	HEIndex outgoingEdge; // One HE starting at this vertex (O(1) Vertex Adjacency traversal)

	// Custom data needed for Catmull-Clark
	XMFLOAT3 vertexPoint; // The newly calculated New Vertex Point (VP')
};

// 3. HFace (References the Original faces list, or is newly created)
struct HFace {
	HEIndex boundingEdge; // One HE belonging to this face
	int originalFaceIndex; // The index of the Face object in QuadSystem::faces[]

	// Custom data needed for Catmull-Clark
	XMFLOAT3 facePoint;   // The newly calculated Face Point (FP)
};

// The central data structure for the subdivision phase
struct HalfEdgeSystem {
	std::vector<HalfEdge> edges;
	std::vector<HVertex> vertices;
	std::vector<HFace> faces;
};


// --- Helper Struct for HEDS Construction ---
// Required for the std::map used in buildAHES to find twin half-edges.
struct EdgePair {
	int v1;
	int v2;
	// Overload comparison operators for use in std::map (required for map key)
	bool operator<(const EdgePair& other) const {
		// Sorts based on v1 first, then v2 if v1's are equal
		return (v1 < other.v1) || (v1 == other.v1 && v2 < other.v2);
	}
};

// Helper function to create the canonical (sorted) EdgePair
inline EdgePair makeCanonicalEdgePair(int p1, int p2) {
	if (p1 > p2) {
		return { p2, p1 };
	}
	return { p1, p2 };
}

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
	void triangulateNgon(uint64_t faceId);
	XMVECTOR calcNormal(int p1, int p2, int p3);
	XMVECTOR calcNormal(int faceIndex);
	//XMVECTOR calcNormal(uint64_t faceId);
	void assignNormal(Face& f);
	std::tuple<float, float> XYZtoUV(XMFLOAT3);
	XMFLOAT3 UVtoXYZ(Face face, std::tuple<float, float> uvCoord);

	XMFLOAT3 findCentroid(Face face);
	XMFLOAT3 findNgonCentroid(const std::vector<XMFLOAT3>& points);
	XMFLOAT3 findCentroid(const std::array<int, 4>& points);
	XMVECTOR findCentroidV(const std::array<XMVECTOR, 4>& points);
	XMVECTOR findNgonCentroidV(const std::vector<XMVECTOR>& points);

	XMFLOAT3 edgeMidpoint(const XMFLOAT3& p1, const XMFLOAT3& p2);
	float distance(XMFLOAT3 pt1, XMFLOAT3 pt2);
	float approxWidth(Face f);
	float approxWidth(std::array<int, 4> corners);
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

	void trackNewBranch(Branch& b, int currentIndex) {
		// Assign unique ID and increment the counter
		b.id = nextBranchID++;
		// Map the ID to its current location
		branchIdToIndexMap[b.id] = currentIndex;
	}	

	int getBranchIndexByID(int branchId) const {
		// Use .at() throws an exception if the ID doesn't exist.
		return branchIdToIndexMap.at(branchId);
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

	void buildCylinder(XMFLOAT3 centerBase, XMFLOAT3 normal, float length, float baseRadius, float taper, int hDivs, int rDivs);

	void buildPlane(int xCount, int zCount);
	

	// replace a face with a box
	//void replaceFace(int faceIndex, XMFLOAT3 normal, float length, XMFLOAT3 endScale, bool cap);
	void extrude(uint64_t faceId, XMFLOAT3 normal, float length, XMFLOAT3 endScale, bool cap, bool isBranch);
	void extrude(std::vector<int> startCorners, XMFLOAT3 normal, float length, XMFLOAT3 endScale, bool cap, bool isBranch);

	// caps
	uint64_t topCap(Box& box);
	uint64_t addCap(Box& box);
	void bottomCap(Box& box);

	uint64_t topCylinderCap(Cylinder& cyl);
	uint64_t bottomCylinderCap(Cylinder& cyl);

	// connections
	void connectBoxes(Box box1, Box box2);

	// premade shapes
	void buildBall(uint64_t faceId, float scale, bool cap, bool isBranch);
	void buildBall(std::array<int, 4> startVerts, float scale, bool cap, bool isBranch);
	void buildBall(std::vector<int> startVerts, float scale, bool cap, bool isBranch);
	void bend(uint64_t faceId, int numSections, XMFLOAT3 angle, float sectionScale);
	void buildFin(uint64_t faceId, int numSections, bool pointDown);
	void bend(std::array<int, 4> nearCorners, int numSections, XMFLOAT3 angle, float sectionScale);
	void bend(std::vector<int> nearCorners, int numSections, XMFLOAT3 angle, float sectionScale);
	void buildFin(std::array<int, 4> nearCorners, float scale, int numSections, bool pointDown);


	// given a point in space and a normal vector, returns a list of 4 orthogonal unit vectors
	std::array<XMFLOAT3, 4> orthoUnits(XMFLOAT3 location, XMVECTOR normal);

	// deconstructs boxes to fill points, lines and faces lists
	void fillLists();

	// subdivision alorithms
	void buildAHES(HalfEdgeSystem& heSystem);
	void loopSubdivide();
	void CCsubdivide(float paramA, float paramB, float paramC);
	void CCsubdivideHE1(float paramA, float paramB, float paramC);
	void CCsubdivideHE2(float paramA, float paramB, float paramC);
	
	void CCsubdivideNgon(float paramA, float paramB, float paramC);
	void CCsubdivideNgonLERP(float paramA, float paramB, float paramC, float strength);

	// two perpendicular cuts, one face becomes 4
	void divideFace(int faceIndex, float dim1, float dim2, float u, float v);
	// parallel cuts
	void sliceFace(int faceIndex, float edge1, int numSections);

	int branch(Branch& parent, int faceId, std::vector<int> sides,  std::vector<float> angles, float boxHeightRatio);

	std::vector<uint64_t> branch(Branch& parent, 
				int faceId, 
				std::vector<int> sides, 
				std::vector<float> angles, 
				float boxHeightRatio, 
				float branchLengthRatio, 
				bool polarity);

	void branchSystem();

	// A and B two  XMVECTORs
	XMFLOAT3 rotateVector(XMVECTOR A, XMVECTOR B, float angle);

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

	Cylinder& getCylinder(int index)
	{
		return cylinders[index];
	}

	Face& getFace(int index)
	{
		return faces[index];
	}

	Face& getNewestFace()
	{
		return faces.back();
	}

	Face& getFaceById(uint64_t faceId)
	{
		int index = getFaceIndexByID(faceId);
		return faces[index];
	}

	void deleteFace(int index)
	{
		faces.erase(faces.begin() + index);
	}

	std::array<int, 4> getFaceVertices(uint64_t faceId)
	{
		int index = getFaceIndexByID(faceId);
		Face& f = faces[index];
		std::array<int, 4> verts = { f.face[0], f.face[3], f.face[2], f.face[1] };
		return verts;
	}

	void deleteStagedFaces();

	XMFLOAT3 basePoint = XMFLOAT3(0.0f, 0.0f, 0.0f);
	uint64_t topCapId;
	uint64_t bottomCapId;
	std::vector<uint64_t> branchCaps;
	std::vector<uint64_t> capIds;

	std::array<XMVECTOR, 4> projectQuad(FXMVECTOR P1, FXMVECTOR P2, FXMVECTOR P3, FXMVECTOR P4, FXMVECTOR V, float L);
	std::array<XMVECTOR, 4> projectQuad(std::array<XMVECTOR, 4> originalPoints, FXMVECTOR V, float L);

private:
	uint64_t nextFaceID = 0;
	int nextBranchID = 0;
	std::vector<XMFLOAT3> points;
	std::vector<Face> faces;
	std::vector<std::pair<int, int>> edges;
	std::vector<std::vector<std::pair<int, int>>> faceEdgePairs;
	std::vector<Quad> quads;
	std::vector<Box> boxes;
	std::vector<Branch> branches;
	std::vector<Cylinder> cylinders;
	std::unordered_map<int, int> branchIdToIndexMap;
	std::unordered_map<uint64_t, int> faceIdToIndexMap;
	std::vector<uint64_t> faceDeletionIdList;
	
	

	// standard indexing for quad->triangle conversion
	std::array<int, 6> triIndices = { 0, 1, 3, 3, 1, 2 };

	XMFLOAT3 origin = XMFLOAT3(0.0f, 0.0f, 0.0f);
};
