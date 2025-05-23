#pragma once

#include <DirectXMath.h>

struct Vertex
{
	Vertex() : pos(0.f, 0.f, 0.f), color(1.f, 1.f, 1.f), normal(0.f, 0.f, 0.f),
		normalV(DirectX::XMLoadFloat3(&normal)) 
		{}

	Vertex(float x, float y, float z)
		: pos(x, y, z),
		color(1.f, 1.f, 1.f),
		normal(0.f, 0.f, 0.f),
		normalV(DirectX::XMLoadFloat3(&normal))
		{}

	Vertex(float x, float y, float z, float r, float g, float b)
		: pos(x, y, z), 
		color(r, g, b), 
		normal(0.f, 0.f, 0.f),
		normalV(DirectX::XMLoadFloat3(&normal))
		{}

	void assign(float x, float y, float z)
	{
		pos.x = x;
		pos.y = y;
		pos.z = z;
	}

	DirectX::XMVECTOR getNormVec()
	{
		return XMLoadFloat3(&normal);
	}

	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 color;
	DirectX::XMFLOAT3 normal;
	DirectX::XMVECTOR normalV;
};
