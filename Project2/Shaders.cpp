#include "Shaders.h"

/////////////////////
//    Vertex Shader

bool VertexShader::Initialize(Microsoft::WRL::ComPtr<ID3D11Device>& device, 
							std::wstring shaderpath, 
							D3D11_INPUT_ELEMENT_DESC* layoutDesc, 
							UINT numElements)
{
	HRESULT hr = D3DReadFileToBlob(shaderpath.c_str(), this->shader_buffer.GetAddressOf());
	if (FAILED(hr))
	{
		std::wstring errorMsg = L"Failed to load shader: ";
		errorMsg += shaderpath;
		EngineException::Log(hr, errorMsg);
		return false;
	}

	hr = device->CreateVertexShader(this->shader_buffer->GetBufferPointer(), 
									this->shader_buffer->GetBufferSize(), 
									NULL, 
									this->shader.GetAddressOf());
	if (FAILED(hr))
	{
		std::wstring errorMsg = L"Failed to create vertex shader: ";
		errorMsg += shaderpath;
		EngineException::Log(hr, errorMsg);
		return false;
	}

	hr = device->CreateInputLayout(layoutDesc, 
								numElements,
								GetBuffer()->GetBufferPointer(), 
								GetBuffer()->GetBufferSize(),
								inputLayoutP.GetAddressOf());

	IF_COM_FAIL(hr, "input layout");

	return true;
}

ID3D11VertexShader* VertexShader::GetShader()
{
	return this->shader.Get();
}

ID3D10Blob* VertexShader::GetBuffer()
{
	return this->shader_buffer.Get();
}

ID3D11InputLayout* VertexShader::GetInputLayout()
{
	return this->inputLayoutP.Get();
}


/////////////////////
//    Pixel Shader

bool PixelShader::Initialize(Microsoft::WRL::ComPtr<ID3D11Device>& device, std::wstring shaderpath)
{
	HRESULT hr = D3DReadFileToBlob(shaderpath.c_str(), this->shader_buffer.GetAddressOf());
	if (FAILED(hr))
	{
		std::wstring errorMsg = L"Failed to load shader: ";
		errorMsg += shaderpath;
		EngineException::Log(hr, errorMsg);
		return false;
	}

	hr = device->CreatePixelShader(this->shader_buffer->GetBufferPointer(), 
									this->shader_buffer->GetBufferSize(), 
									NULL, 
									this->shader.GetAddressOf());
	if (FAILED(hr))
	{
		std::wstring errorMsg = L"Failed to create vertex shader: ";
		errorMsg += shaderpath;
		EngineException::Log(hr, errorMsg);
		return false;
	}

	return true;
}

ID3D11PixelShader* PixelShader::GetShader()
{
	return this->shader.Get();
}

ID3D10Blob* PixelShader::GetBuffer()
{
	return this->shader_buffer.Get();
}