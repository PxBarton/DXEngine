#pragma once

#include <DirectXMath.h>
#include <cmath>
#include <vector>
#include <array>
#include "Vertex.h"
#include "Mesh.h"


// represents a flat quad, eventually subdivided and/or triangulated, tri's inherit normal, if calculated
// position data alone is sufficient for further operations, like subQuad and extrusion
// normal plus a center point yield an axis for extrusion and cylindrical coords
// use for redundant indexing (real flat shading) 
struct Quad
{
	XMFLOAT3 v1;
	XMFLOAT3 v2;
	XMFLOAT3 v3;
	XMFLOAT3 v4;

	DirectX::XMVECTOR normalV;
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
	std::vector<int> triangulate();
	
	void loopSubdivide();
	void CCsubdivide();
	void subQuad(Quad q, float dim1, float dim2, float u, float v);

	// standard indexing
	std::unique_ptr<Mesh> convertToMesh1();
	// redundant indexing
	std::unique_ptr<Mesh> convertToMesh2();

	// functions to define 3D structural components and random/procedural systems
	void buildFlatSquare(float length);

private:
	std::unique_ptr<XMFLOAT3[]> quadArray = nullptr;
	std::vector<Quad> quads;
	//standard indexing
	std::array<int, 6> triIndices = { 0, 3, 1, 1, 3, 2 };

	XMFLOAT3 origin = XMFLOAT3(0.0f, 0.0f, 0.0f);
};
