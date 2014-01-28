// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once

#include "ShaderManager.h"

class Material;

class MaterialShaderManager : public ShaderManager
{
public:
	MaterialShaderManager(ID3D11Device* device);
	~MaterialShaderManager();

	ID3D11VertexShader* GetVertexShader(const std::string& file, const std::string& entryPoint, const std::string& model, const Material& material);
	ID3D11GeometryShader* GetGeometryShader(const std::string& file, const std::string& entryPoint, const std::string& model, const Material& material);
	ID3D11PixelShader* GetPixelShader(const std::string& file, const std::string& entryPoint, const std::string& model, const Material& material);

	bool GetBlob(const std::string& file, const std::string& entryPoint, const std::string& model, const Material& material, ID3DBlob** blob);

private:
	void BuildPrologue(const Material& material, std::ostringstream& prologue);

	template<typename T, typename Y>
	bool FindShader(const std::string& hash, const Material& material, T& collection, Y** result);

	typedef std::unordered_map<unsigned, ID3D11VertexShader*> VSMaterialMap;
	typedef std::unordered_map<unsigned, ID3D11GeometryShader*> GSMaterialMap;
	typedef std::unordered_map<unsigned, ID3D11PixelShader*> PSMaterialMap;
	std::unordered_map<std::string, VSMaterialMap> m_VertexShaders;
	std::unordered_map<std::string, GSMaterialMap> m_GeometryShaders;
	std::unordered_map<std::string, PSMaterialMap> m_PixelShaders;
};