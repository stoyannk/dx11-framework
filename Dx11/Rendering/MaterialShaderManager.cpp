// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#include "stdafx.h"

#include "MaterialShaderManager.h"
#include "Material.h"

using namespace DirectX;

MaterialShaderManager::MaterialShaderManager(ID3D11Device* device)
	: ShaderManager(device)
{}

MaterialShaderManager::~MaterialShaderManager()
{
	std::for_each(m_VertexShaders.begin(), m_VertexShaders.end(), [] (std::pair<const std::string, VSMaterialMap>& matMap)
	{
		std::for_each(matMap.second.begin(), matMap.second.end(), [] (std::pair<const unsigned, ID3D11VertexShader*>& vs)
		{
			vs.second->Release();
		});
	});

	std::for_each(m_GeometryShaders.begin(), m_GeometryShaders.end(), [] (std::pair<const std::string, GSMaterialMap>& matMap)
	{
		std::for_each(matMap.second.begin(), matMap.second.end(), [] (std::pair<const unsigned, ID3D11GeometryShader*>& gs)
		{
			gs.second->Release();
		});
	});

	std::for_each(m_PixelShaders.begin(), m_PixelShaders.end(), [] (std::pair<const std::string, PSMaterialMap>& matMap)
	{
		std::for_each(matMap.second.begin(), matMap.second.end(), [] (std::pair<const unsigned, ID3D11PixelShader*>& ps)
		{
			ps.second->Release();
		});
	});
}

bool MaterialShaderManager::GetBlob(const std::string& file, const std::string& entryPoint, const std::string& model, const Material& material, ID3DBlob** blob)
{
	std::ostringstream prologue;
	BuildPrologue(material, prologue);

	if(!CompileShaderFromFile(file, entryPoint, model, blob, prologue.str()))
	{
		return false;
	}

	return true;
}

ID3D11VertexShader* MaterialShaderManager::GetVertexShader(const std::string& file, const std::string& entryPoint, const std::string& model, const Material& material)
{
	// Check if the shader is in cache
	std::string toHash;
	toHash.reserve(file.size() + entryPoint.size() + model.size());
	toHash.append(file);
	toHash.append(entryPoint);
	toHash.append(model);

	ID3D11VertexShader* vs = nullptr;
	if(FindShader(toHash, material, m_VertexShaders, &vs))
	{
		return vs;
	}
	// Either there is no such shader or none for the material - so compile it
	std::ostringstream prologue;
	BuildPrologue(material, prologue);

	ID3DBlob* shader = nullptr;
	if(!CompileShaderFromFile(file, entryPoint, model, &shader, prologue.str()))
	{
		return nullptr;
	}

	auto ret = CreateVertexShader(shader, nullptr);
	SafeRelease(shader);

	m_VertexShaders[toHash].insert(std::make_pair(material.GetProperties(), ret));

	return ret;
}

ID3D11GeometryShader* MaterialShaderManager::GetGeometryShader(const std::string& file, const std::string& entryPoint, const std::string& model, const Material& material)
{
	// Check if the shader is in cache
	std::string toHash;
	toHash.reserve(file.size() + entryPoint.size() + model.size());
	toHash.append(file);
	toHash.append(entryPoint);
	toHash.append(model);

	ID3D11GeometryShader* gs = nullptr;
	if(FindShader(toHash, material, m_GeometryShaders, &gs))
	{
		return gs;
	}
	// Either there is no such shader or none for the material - so compile it
	std::ostringstream prologue;
	BuildPrologue(material, prologue);

	ID3DBlob* shader = nullptr;
	if(!CompileShaderFromFile(file, entryPoint, model, &shader, prologue.str()))
	{
		return nullptr;
	}

	auto ret = CreateGeometryShader(shader, nullptr);
	SafeRelease(shader);

	m_GeometryShaders[toHash].insert(std::make_pair(material.GetProperties(), ret));

	return ret;
}

ID3D11PixelShader* MaterialShaderManager::GetPixelShader(const std::string& file, const std::string& entryPoint, const std::string& model, const Material& material)
{
	// Check if the shader is in cache
	std::string toHash;
	toHash.reserve(file.size() + entryPoint.size() + model.size());
	toHash.append(file);
	toHash.append(entryPoint);
	toHash.append(model);

	ID3D11PixelShader* ps = nullptr;
	if(FindShader(toHash, material, m_PixelShaders, &ps))
	{
		return ps;
	}
	// Either there is no such shader or none for the material - so compile it
	std::ostringstream prologue;
	BuildPrologue(material, prologue);

	ID3DBlob* shader = nullptr;
	if(!CompileShaderFromFile(file, entryPoint, model, &shader, prologue.str()))
	{
		return nullptr;
	}
	
	auto ret = CreatePixelShader(shader, nullptr);
	SafeRelease(shader);

	m_PixelShaders[toHash].insert(std::make_pair(material.GetProperties(), ret));

	return ret;
}
 
template<typename T, typename Y>
bool MaterialShaderManager::FindShader(const std::string& hash, const Material& material, T& collection, Y** result)
{
	auto fileIt = collection.find(hash);

	// There is such a shader
	if(fileIt != collection.end())
	{
		// Check if there is a combination for the material
		auto shaderIt = fileIt->second.find(material.GetProperties());
		if(shaderIt != fileIt->second.end())
		{
			*result = shaderIt->second;
			return true;
		}
		else 
		{
			return false;
		}
	}
	
	return false;
}

#define INJECT_MATERIAL_PROP(PROPERTY, NAME) \
if (material.HasProperty(PROPERTY)) \
{ \
	options << "static const bool " NAME " = true; \n"; \
} \
else \
{ \
	options << "static const bool " NAME " = false; \n"; \
}\

void MaterialShaderManager::BuildPrologue(const Material& material, std::ostringstream& prologue)
{
	std::ostringstream options;

	INJECT_MATERIAL_PROP(MP_Diffuse, "g_HasDiffuse")
	INJECT_MATERIAL_PROP(MP_Normal, "g_HasNormal")
	INJECT_MATERIAL_PROP(MP_AlphaMask, "g_HasAlphaMask")
	INJECT_MATERIAL_PROP(MP_SpecularMap, "g_HasSpecularMap")
	INJECT_MATERIAL_PROP(MP_SpecularPower, "g_HasSpecularPower")

	prologue << options.str();
}