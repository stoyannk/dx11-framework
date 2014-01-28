// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once

enum MTL_ERROR
{
	MTL_OK,
	MTL_FILENOTFOUND,
	MTL_PARSE_ERROR
};

class MtlLoader
{
public:
	struct MaterialTextures
	{
		std::string Diffuse;
		std::string NormalMap;
		std::string Mask;
		std::string Specular;
		float SpecularPower;
		DirectX::XMFLOAT3 DiffuseColor;
	};

	MtlLoader(const std::string& filename);
	bool IsOpen();
	MTL_ERROR GetLastError();
	MaterialTextures GetMaterialInfo(const std::string& material);

private:
	bool m_IsOpen;
	MTL_ERROR m_Error;

	typedef std::map<std::string, MaterialTextures> MaterialsMap;
	MaterialsMap m_Materials;
};