#pragma once

#include "Vertex.h"
#include "Buffers.h"

using namespace DirectX;

class SceneObject
{
public:
	
	const XMVECTOR& getPositionVector() const;
	const XMFLOAT3& getPositionFloat3() const;
	const XMVECTOR& getRotationVector() const;
	const XMFLOAT3& getRotationFloat3() const;

	
	void initPosition(float x, float y, float z);
	void setPosition(float x, float y, float z);
	void initRotation(float x, float y, float z);
	void setRotation(float x, float y, float z);
	void SetLookAtPos(XMFLOAT3 lookAtPos);

protected:
	XMMATRIX worldMatrix = XMMatrixIdentity();

	void UpdateMatrix();
	void UpdateDirectionVectors();

	XMVECTOR positionVec;
	XMVECTOR rotationVec;
	XMFLOAT3 position;
	XMFLOAT3 rotation;

	
};
