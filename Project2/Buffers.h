#pragma once

#ifndef VertexBuffer_h__
#define VertexBuffer_h__

#include "EngineException.h"

#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <vector>


////////////////////////////////////////////////
////    Vertex Buffer    ///////////////////////

template<class T>
class VertexBuffer
{
private:
	VertexBuffer(const VertexBuffer<T>& rhs);

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
	std::unique_ptr<UINT> stride;
	UINT bufferSize = 0;

public:
	VertexBuffer() {}

	ID3D11Buffer* Get()const
	{
		return buffer.Get();
	}

	ID3D11Buffer* const* GetAddressOf()const
	{
		return buffer.GetAddressOf();
	}

	UINT BufferSize() const
	{
		return this->bufferSize;
	}

	const UINT Stride() const
	{
		return *this->stride.get();
	}

	const UINT* StridePtr() const
	{
		return this->stride.get();
	}

	// T* data
	HRESULT Initialize(ID3D11Device* device, T* data, UINT numVertices)
	{
		if (buffer.Get() != nullptr)
		{
			buffer.Reset();
		}

		this->bufferSize = numVertices;

		if (this->stride.get() == nullptr)
		{
			this->stride = std::make_unique<UINT>(sizeof(T));
		}
		

		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof(T) * numVertices;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA vertexBufferData;
		ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
		vertexBufferData.pSysMem = data;

		HRESULT hr = device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, this->buffer.GetAddressOf());
		return hr;
	}
};

#endif // VertexBuffer_h__


////////////////////////////////////////////////
////    Index Buffer    ////////////////////////

#ifndef IndicesBuffer_h__
#define IndicesBuffer_h__

class IndexBuffer
{
private:
	IndexBuffer(const IndexBuffer& rhs);

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
	UINT bufferSize = 0;
public:
	IndexBuffer() {}

	ID3D11Buffer* Get()const
	{
		return buffer.Get();
	}

	ID3D11Buffer* const* GetAddressOf()const
	{
		return buffer.GetAddressOf();
	}

	UINT BufferSize() const
	{
		return this->bufferSize;
	}

	// DWORD* data
	HRESULT Initialize(ID3D11Device* device, DWORD* data, UINT numIndices)
	{
		if (buffer.Get() != nullptr)
		{
			buffer.Reset();
		}

		this->bufferSize = numIndices;
		//Load Index Data
		D3D11_BUFFER_DESC indexBufferDesc;
		ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = sizeof(DWORD) * numIndices;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA indexBufferData;
		indexBufferData.pSysMem = data;
		HRESULT hr = device->CreateBuffer(&indexBufferDesc, &indexBufferData, buffer.GetAddressOf());
		return hr;
	}
};

#endif // IndicesBuffer_h__


////////////////////////////////////////////////
////    Constant Buffer    /////////////////////


#ifndef ConstantBuffer_h__
#define ConstantBuffer_h__


// types

struct CB_VS_vertexshader
{
	float xOffset = 0; //4bytes
	float yOffset = 0; //4bytes
	DirectX::XMMATRIX mat;
};

struct CB_PS_light
{
	DirectX::XMFLOAT3 ambientLightColor;
	float ambientLightStrength;

	DirectX::XMFLOAT3 dynamicLightColor;
	float dynamicLightStrength;
	DirectX::XMFLOAT3 dynamicLightPosition;
	float dynamicLightAttenuation_a;
	float dynamicLightAttenuation_b;
	float dynamicLightAttenuation_c;
};


// const buffer template class

template<class T>
class ConstantBuffer
{
private:
	ConstantBuffer(const ConstantBuffer<T>& rhs);

private:

	Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
	ID3D11DeviceContext* deviceContext = nullptr;

public:
	ConstantBuffer() {}

	T data;

	ID3D11Buffer* Get()const
	{
		return buffer.Get();
	}

	ID3D11Buffer* const* GetAddressOf()const
	{
		return buffer.GetAddressOf();
	}

	// pass deviceP.Get(), deviceContextP.Get() as arguments
	HRESULT Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
	{
		if (buffer.Get() != nullptr)
		{
			buffer.Reset();
		}

		// assign graphics::deviceContextP to deviceContext
		this->deviceContext = deviceContext;

		D3D11_BUFFER_DESC desc;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;
		desc.ByteWidth = static_cast<UINT>(sizeof(T) + (16 - (sizeof(T) % 16)));
		desc.StructureByteStride = 0;

		HRESULT hr = device->CreateBuffer(&desc, 0, buffer.GetAddressOf());
		return hr;
	}

	// actually populates the ID3D11 buffer with the content of data 
	bool ApplyChanges()
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		HRESULT hr = this->deviceContext->Map(buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (FAILED(hr))
		{
			EngineException::Log(hr, "Failed to map constant buffer.");
			return false;
		}
		CopyMemory(mappedResource.pData, &data, sizeof(T));
		this->deviceContext->Unmap(buffer.Get(), 0);
		return true;
	}
};

#endif // ConstantBuffer_h__