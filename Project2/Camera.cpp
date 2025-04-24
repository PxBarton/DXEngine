#include "Camera.h"

Camera::Camera()
{
	this->pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	this->posVector = XMLoadFloat3(&this->pos);
	this->rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	this->rotVector = XMLoadFloat3(&this->rot);
	this->UpdateViewMatrix();
}

void Camera::SetProjectionValues(float fovDegrees, float aspectRatio, float nearZ, float farZ)
{
	float fovRadians = (fovDegrees / 360.0f) * XM_2PI;
	this->projectionMatrix = XMMatrixPerspectiveFovLH(fovRadians, aspectRatio, nearZ, farZ);
}

const XMMATRIX& Camera::GetViewMatrix() const
{
	return this->viewMatrix;
}

const XMMATRIX& Camera::GetProjectionMatrix() const
{
	return this->projectionMatrix;
}

const XMVECTOR& Camera::GetPositionV() const
{
	return this->posVector;
}

const XMFLOAT3& Camera::GetPositionF() const
{
	return this->pos;
}

const XMVECTOR& Camera::GetRotationV() const
{
	return this->rotVector;
}

const XMFLOAT3& Camera::GetRotationF() const
{
	return this->rot;
}

void Camera::SetPosition(const XMVECTOR& pos)
{
	XMStoreFloat3(&this->pos, pos);
	this->posVector = pos;
	this->UpdateViewMatrix();
}

void Camera::SetPosition(float x, float y, float z)
{
	this->pos = XMFLOAT3(x, y, z);
	this->posVector = XMLoadFloat3(&this->pos);
	this->UpdateViewMatrix();
}

void Camera::AdjustPosition(const XMVECTOR& pos)
{
	this->posVector += pos;
	XMStoreFloat3(&this->pos, this->posVector);
	this->UpdateViewMatrix();
}

void Camera::AdjustPosition(float x, float y, float z)
{
	this->pos.x += x;
	this->pos.y += y;
	this->pos.z += z;
	this->posVector = XMLoadFloat3(&this->pos);
	this->UpdateViewMatrix();
}

void Camera::SetRotation(const XMVECTOR& rot)
{
	this->rotVector = rot;
	XMStoreFloat3(&this->rot, rot);
	this->UpdateViewMatrix();
}

void Camera::SetRotation(float x, float y, float z)
{
	this->rot = XMFLOAT3(x, y, z);
	this->rotVector = XMLoadFloat3(&this->rot);
	this->UpdateViewMatrix();
}

void Camera::AdjustRotation(const XMVECTOR& rot)
{
	this->rotVector += rot;
	XMStoreFloat3(&this->rot, this->rotVector);
	this->UpdateViewMatrix();
}



void Camera::AdjustRotation(float x, float y, float z)
{
	this->rot.x += x;
	this->rot.y += y;
	this->rot.z += z;
	this->rotVector = XMLoadFloat3(&this->rot);
	this->UpdateViewMatrix();
}

void Camera::SetLookAtPos(XMFLOAT3 lookAtPos)
{
	// Verify that look at pos != cam pos
	if (lookAtPos.x == this->pos.x && lookAtPos.y == this->pos.y && lookAtPos.z == this->pos.z)
		return;

	lookAtPos.x = this->pos.x - lookAtPos.x;
	lookAtPos.y = this->pos.y - lookAtPos.y;
	lookAtPos.z = this->pos.z - lookAtPos.z;

	float pitch = 0.0f;
	if (lookAtPos.y != 0.0f)
	{
		const float distance = sqrt(lookAtPos.x * lookAtPos.x + lookAtPos.z * lookAtPos.z);
		pitch = atan(lookAtPos.y / distance);
	}

	float yaw = 0.0f;
	if (lookAtPos.x != 0.0f)
	{
		yaw = atan(lookAtPos.x / lookAtPos.z);
	}
	if (lookAtPos.z > 0)
		yaw += XM_PI;

	this->SetRotation(pitch, yaw, 0.0f);
}

const XMVECTOR& Camera::GetForwardV()
{
	return this->forwardV;
}

const XMVECTOR& Camera::GetRightV()
{
	return this->rightV;
}

const XMVECTOR& Camera::GetBackwardV()
{
	return this->backV;
}

const XMVECTOR& Camera::GetLeftV()
{
	return this->leftV;
}

void Camera::UpdateViewMatrix() //Updates view matrix and direction vectors
{
	// XMVector3TransformCoord ignores the w component of the input vector, and uses a value of 1.0 instead. 
	// The w component of the returned vector will always be 1.0.
	
	// Calculate camera rotation matrix
	XMMATRIX camRotationMatrix = XMMatrixRotationRollPitchYaw(this->rot.x, this->rot.y, this->rot.z);
	// Calculate unit vector of cam target based off camera forward value transformed by cam rotation matrix
	XMVECTOR camTarget = XMVector3TransformCoord(this->defaultForwardV, camRotationMatrix);
	// Adjust cam target to be offset by the camera's current position
	camTarget += this->posVector;
	// Calculate up direction based on current rotation
	XMVECTOR upDir = XMVector3TransformCoord(this->defaultUpV, camRotationMatrix);
	// Rebuild view matrix
	this->viewMatrix = XMMatrixLookAtLH(this->posVector, camTarget, upDir);

	XMMATRIX vecRotationMatrix = XMMatrixRotationRollPitchYaw(0.0f, this->rot.y, 0.0f);
	this->forwardV = XMVector3TransformCoord(this->defaultForwardV, vecRotationMatrix);
	this->backV = XMVector3TransformCoord(this->defaultBackV, vecRotationMatrix);
	this->leftV = XMVector3TransformCoord(this->defaultLeftV, vecRotationMatrix);
	this->rightV = XMVector3TransformCoord(this->defaultRightV, vecRotationMatrix);
}

// should be using quaternions

// XMVECTOR XM_CALLCONV XMVector3Rotate(
// [in] FXMVECTOR V,
//	[in] FXMVECTOR RotationQuaternion
//	) noexcept;
