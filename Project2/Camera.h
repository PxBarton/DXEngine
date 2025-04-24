#pragma once

#include <DirectXMath.h>
using namespace DirectX;


class Camera
{
public:
	Camera();
	XMFLOAT3 origin = XMFLOAT3(0.0f, 0.0f, 0.0f);
	void SetProjectionValues(float fovDegrees, float aspectRatio, float nearZ, float farZ);

	const XMMATRIX& GetViewMatrix() const;
	const XMMATRIX& GetProjectionMatrix() const;

	const XMVECTOR& GetPositionV() const;
	const XMFLOAT3& GetPositionF() const;
	const XMVECTOR& GetRotationV() const;
	const XMFLOAT3& GetRotationF() const;

	void SetPosition(const XMVECTOR& pos);
	void SetPosition(float x, float y, float z);

	void AdjustPosition(const XMVECTOR& pos);
	void AdjustPosition(float x, float y, float z);

	void SetRotation(const XMVECTOR& rot);
	void SetRotation(float x, float y, float z);

	void AdjustRotation(const XMVECTOR& rot);
	void AdjustRotation(float x, float y, float z);

	void SetLookAtPos(XMFLOAT3 lookAtPos);
	const XMVECTOR& GetForwardV();
	const XMVECTOR& GetRightV();
	const XMVECTOR& GetBackwardV();
	const XMVECTOR& GetLeftV();

private:
	void UpdateViewMatrix();
	XMVECTOR posVector;
	XMVECTOR rotVector;
	XMFLOAT3 pos;
	XMFLOAT3 rot;
	
	XMMATRIX viewMatrix;
	XMMATRIX projectionMatrix;

	const XMVECTOR defaultForwardV = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	const XMVECTOR defaultUpV = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	const XMVECTOR defaultBackV = XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
	const XMVECTOR defaultLeftV = XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f);
	const XMVECTOR defaultRightV = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

	XMVECTOR forwardV;
	XMVECTOR backV;
	XMVECTOR rightV;
	XMVECTOR leftV;
};

// XMVECTOR and XMMATRIX are for computation
// XMFLOAT is for storage 
// load -> XMVECTOR, XMMATRIX
// store -> XMFLOAT
