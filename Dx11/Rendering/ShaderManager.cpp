// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#include "stdafx.h"
#include <D3DCompiler.h>
#include "ShaderManager.h"

ShaderManager::ShaderManager(ID3D11Device* device)
	: m_Device(device)
{
}

bool ShaderManager::CompileShaderFromFile(const std::string& fileName
										, const std::string& entryPoint
										, const std::string& shaderModel
										, ID3DBlob** shader
										, const std::string& prologue)
{
	HRESULT hr = S_OK;

	DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

#if defined(DEBUG) || defined(_DEBUG)
    shaderFlags |= D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
	shaderFlags |= D3DCOMPILE_PREFER_FLOW_CONTROL;
#endif

    ID3DBlob* errorBlob = nullptr;
	
	// read the file data
	std::ifstream fin(fileName.c_str());

	if(!fin.is_open()) return false;

	fin.seekg(0, std::ios::end);
	size_t len = (size_t)fin.tellg();
	fin.seekg(0, std::ios::beg);

	const size_t prologueSz = prologue.size();
	len += prologueSz;

	boost::scoped_array<char> data(new char[len]);

	strcpy(data.get(), prologue.c_str());
	fin.read(data.get() + prologueSz, len - prologueSz);

	len = size_t(fin.gcount() + prologueSz);

	hr = D3DCompile((const void*)data.get(), len, fileName.c_str(), nullptr, nullptr, 
					entryPoint.c_str(), shaderModel.c_str(), shaderFlags, 0, shader, &errorBlob);

	ReleaseGuard<ID3DBlob> errorGuard(errorBlob);
    
	if(FAILED(hr))
    {
        if(errorBlob != nullptr)
		{
			std::string error((char*)errorGuard.Get()->GetBufferPointer());
			SLOG(Sev_Error, Fac_Rendering, error);
		}
        return false;
    }
    
    return true;
}

ID3D11VertexShader* ShaderManager::CreateVertexShader(ID3DBlob* compiledCode, ID3D11ClassLinkage* linkage)
{
	HRESULT hr = S_OK;

	ID3D11VertexShader* shader = nullptr;

	hr = m_Device->CreateVertexShader(compiledCode->GetBufferPointer(), compiledCode->GetBufferSize(), linkage, &shader);

	if(FAILED(hr))
	{
		SLOG(Sev_Error, Fac_Rendering, "Unable to create Vertex Shader");
		return nullptr;
	}

	return shader;
}

ID3D11GeometryShader* ShaderManager::CreateGeometryShader(ID3DBlob* compiledCode, ID3D11ClassLinkage* linkage)
{
	HRESULT hr = S_OK;

	ID3D11GeometryShader* shader = nullptr;

	hr = m_Device->CreateGeometryShader(compiledCode->GetBufferPointer(), compiledCode->GetBufferSize(), linkage, &shader);

	if(FAILED(hr))
	{
		SLOG(Sev_Error, Fac_Rendering, "Unable to create Geometry Shader");
		return nullptr;
	}

	return shader;
}

ID3D11PixelShader* ShaderManager::CreatePixelShader(ID3DBlob* compiledCode, ID3D11ClassLinkage* linkage)
{
	HRESULT hr = S_OK;

	ID3D11PixelShader* shader = nullptr;

	hr = m_Device->CreatePixelShader(compiledCode->GetBufferPointer(), compiledCode->GetBufferSize(), linkage, &shader);

	if(FAILED(hr))
	{
		SLOG(Sev_Error, Fac_Rendering, "Unable to create Pixel Shader");
		return nullptr;
	}

	return shader;
}

ID3D11ComputeShader* ShaderManager::CreateComputeShader(ID3DBlob* compiledCode, ID3D11ClassLinkage* linkage) 
{
	HRESULT hr = S_OK;

	ID3D11ComputeShader* shader = nullptr;

	hr = m_Device->CreateComputeShader(compiledCode->GetBufferPointer(), compiledCode->GetBufferSize(), linkage, &shader);

	if(FAILED(hr))
	{
		SLOG(Sev_Error, Fac_Rendering, "Unable to create Compute Shader");
		return nullptr;
	}

	return shader;
}

ID3D11VertexShader* ShaderManager::CompileVertexShader(const std::string& shadersFileName
											, const std::string& vsEntry
											, const std::string& vsModel)
{
	ID3DBlob* blob = nullptr;
	if(!CompileShaderFromFile(shadersFileName
							, vsEntry
							, vsModel
							, &blob))
	{
		return nullptr;
	}
	ReleaseGuard<ID3DBlob> blobGuard(blob);

	return CreateVertexShader(blobGuard.Get(), nullptr);
}

ID3D11PixelShader* ShaderManager::CompilePixelShader(const std::string& shadersFileName
											, const std::string& psEntry
											, const std::string& psModel)
{
	ID3DBlob* blob = nullptr;
	if(!CompileShaderFromFile(shadersFileName
							, psEntry
							, psModel
							, &blob))
	{
		return nullptr;
	}
	ReleaseGuard<ID3DBlob> blobGuard(blob);

	return CreatePixelShader(blobGuard.Get(), nullptr);
}

ID3D11GeometryShader* ShaderManager::CompileGeometryShader(const std::string& shadersFileName
											, const std::string& gsEntry
											, const std::string& gsModel)
{
	ID3DBlob* blob = nullptr;
	if(!CompileShaderFromFile(shadersFileName
							, gsEntry
							, gsModel
							, &blob))
	{
		return nullptr;
	}
	ReleaseGuard<ID3DBlob> blobGuard(blob);

	return CreateGeometryShader(blobGuard.Get(), nullptr);
}

ID3D11ComputeShader* ShaderManager::CompileComputeShader(const std::string& shadersFileName
											, const std::string& csEntry
											, const std::string& csModel)
{
	ID3DBlob* blob = nullptr;
	if(!CompileShaderFromFile(shadersFileName
							, csEntry
							, csModel
							, &blob))
	{
		return nullptr;
	}
	ReleaseGuard<ID3DBlob> blobGuard(blob);

	return CreateComputeShader(blobGuard.Get(), nullptr);
}
   
bool ShaderManager::CompileShaderDuo(const std::string& shadersFileName
								, const std::string& vsEntry
								, const std::string& vsModel
								, const std::string& psEntry
								, const std::string& psModel
								, CompilationOutput& output)
{
	ID3DBlob* vsBlob = nullptr;
	if(!ShaderManager::CompileShaderFromFile(shadersFileName, vsEntry, vsModel, &vsBlob))
	{
		SLOG(Sev_Error, Fac_Rendering, "Unable to compile vertex shader ", shadersFileName);
		return false;
	}
	output.vsBlob = vsBlob;

	ID3DBlob* psBlob = nullptr;
	if(!ShaderManager::CompileShaderFromFile(shadersFileName, psEntry, psModel, &psBlob))
	{
		SLOG(Sev_Error, Fac_Rendering, "Unable to compile pixel shader ", shadersFileName);
		return false;
	}
	output.psBlob = psBlob;

	// Compile shaders
	ID3D11VertexShader* vertexShader = CreateVertexShader(vsBlob, nullptr);
	if(!vertexShader)
	{
		SLOG(Sev_Error, Fac_Rendering, "Unable to create vertex shader");
		return false;
	}
	output.vertexShader = vertexShader;

	ID3D11PixelShader* pixelShader = CreatePixelShader(psBlob, nullptr);
	if(!pixelShader)
	{
		SLOG(Sev_Error, Fac_Rendering, "Unable to create pixel shader");
		return false;
	}
	output.pixelShader = pixelShader;

	return true;
}

bool ShaderManager::CompileShaderTrio(const std::string& shadersFileName
							, const std::string& vsEntry
							, const std::string& vsModel
							, const std::string& gsEntry
							, const std::string& gsModel
							, const std::string& psEntry
							, const std::string& psModel
							, CompilationOutput& output)
{
	// Compile the VS and PS
	if(!CompileShaderDuo(shadersFileName, vsEntry, vsModel, psEntry, psModel, output))
	{
		return false;
	}

	// Compile the geometry shader
	ID3DBlob* gsBlob = nullptr;
	if(!ShaderManager::CompileShaderFromFile(shadersFileName, gsEntry, gsModel, &gsBlob))
	{
		SLOG(Sev_Error, Fac_Rendering, "Unable to compile geometry shader ", shadersFileName);
		return false;
	}
	output.gsBlob = gsBlob;

	ID3D11GeometryShader* geometryShader = CreateGeometryShader(gsBlob, nullptr);
	if(!geometryShader)
	{
		SLOG(Sev_Error, Fac_Rendering, "Unable to create geometry shader");
		return false;
	}
	output.geometryShader = geometryShader;

	return true;
}


