// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once

class ShaderManager
{
public:
	ShaderManager(ID3D11Device* device);

	ID3D11VertexShader* CreateVertexShader(ID3DBlob* compiledCode, ID3D11ClassLinkage* linkage);

	ID3D11GeometryShader* CreateGeometryShader(ID3DBlob* compiledCode, ID3D11ClassLinkage* linkage);

	ID3D11PixelShader* CreatePixelShader(ID3DBlob* compiledCode, ID3D11ClassLinkage* linkage);

	ID3D11ComputeShader* CreateComputeShader(ID3DBlob* compiledCode, ID3D11ClassLinkage* linkage);


	ID3D11VertexShader* CompileVertexShader(const std::string& shadersFileName
											, const std::string& vsEntry
											, const std::string& vsModel);

	ID3D11PixelShader* CompilePixelShader(const std::string& shadersFileName
											, const std::string& psEntry
											, const std::string& psModel);

	ID3D11GeometryShader* CompileGeometryShader(const std::string& shadersFileName
											, const std::string& gsEntry
											, const std::string& gsModel);

	ID3D11ComputeShader* CompileComputeShader(const std::string& shadersFileName
											, const std::string& csEntry
											, const std::string& csModel);

	struct CompilationOutput
	{
		CompilationOutput()
			: vsBlob(nullptr)
			, vertexShader(nullptr)
			, gsBlob(nullptr)
			, geometryShader(nullptr)
			, psBlob(nullptr)
			, pixelShader(nullptr)
		{}

		void ReleaseAll()
		{
			SafeRelease(vsBlob);
			SafeRelease(vertexShader);
			SafeRelease(gsBlob);
			SafeRelease(geometryShader);
			SafeRelease(psBlob);
			SafeRelease(pixelShader);
		}

		ID3DBlob* vsBlob;
		ID3D11VertexShader* vertexShader;
		ID3DBlob* gsBlob;
		ID3D11GeometryShader* geometryShader;
		ID3DBlob* psBlob;
		ID3D11PixelShader* pixelShader;
	};

	bool CompileShaderDuo(const std::string& shadersFileName
							, const std::string& vsEntry
							, const std::string& vsModel
							, const std::string& psEntry
							, const std::string& psModel
							, CompilationOutput& output);

	bool CompileShaderTrio(const std::string& shadersFileName
							, const std::string& vsEntry
							, const std::string& vsModel
							, const std::string& gsEntry
							, const std::string& gsModel
							, const std::string& psEntry
							, const std::string& psModel
							, CompilationOutput& output);

	static bool CompileShaderFromFile(const std::string& fileName
										, const std::string& entryPoint
										, const std::string& shaderModel
										, ID3DBlob** shader
										, const std::string& prologue = "");

	template<typename T>
	bool CreateEasyConstantBuffer(ID3D11Buffer** buffer, bool isDynamic = false);

	template<typename T>
	bool CreateEasyStructuredBuffer(ID3D11Buffer** buffer, unsigned numElements, bool isDynamic = false, bool isUav = false);

	bool CreateStructuredBuffer(unsigned elementSize, unsigned elementCount, ID3D11Buffer** buffer, ID3D11UnorderedAccessView** uav);

	bool CreateIndexedIndirectBuffer(ID3D11Buffer** buffer, ID3D11UnorderedAccessView** uav);

	bool CreateGeneratedBuffer(unsigned elementSize, unsigned elementCount, ID3D11Buffer** buffer, ID3D11UnorderedAccessView** uav);

protected:
	ID3D11Device* m_Device;
};

template<typename T>
bool ShaderManager::CreateEasyStructuredBuffer(ID3D11Buffer** buffer, unsigned numElements, bool isDynamic, bool isUav)
{
	D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = isDynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(T) * numElements;
    bd.BindFlags = D3D11_BIND_SHADER_RESOURCE | (isUav ? D3D11_BIND_UNORDERED_ACCESS : 0);
	bd.CPUAccessFlags = isDynamic ? D3D11_CPU_ACCESS_WRITE : 0;
	bd.StructureByteStride = sizeof(T);

	HRESULT hr = m_Device->CreateBuffer(&bd, nullptr, buffer);
    
	if(FAILED(hr))
	{
		SLOG(Sev_Error, Fac_Rendering, "Unable to create structured buffer");
		return false;
	}

	return true;
}

template<typename T>
bool ShaderManager::CreateEasyConstantBuffer(ID3D11Buffer** buffer, bool isDynamic)
{
	D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = isDynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(T);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = isDynamic ? D3D11_CPU_ACCESS_WRITE : 0;

	HRESULT hr = m_Device->CreateBuffer(&bd, nullptr, buffer);
    
	if(FAILED(hr))
	{
		SLOG(Sev_Error, Fac_Rendering, "Unable to create constant buffer");
		return false;
	}

	return true;
}