// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#include "stdafx.h"
#include "MtlLoader.h"

#ifndef MINIMAL_SIZE

MtlLoader::MtlLoader(const std::string& filename)
	: m_IsOpen(false)
	, m_Error(MTL_OK)
{
	std::ifstream fin(filename.c_str());

	if(!fin.is_open())
	{
		m_Error = MTL_FILENOTFOUND;
		return;
	}

	static const std::string NEWMTL = "newmtl";
	static const std::string TEXTURE = "map_Ka";
	static const std::string BUMP = "map_bump";
	static const std::string MASK = "map_d";
	static const std::string SPECULAR = "map_Ks";
	static const std::string DIFFUSE_COLOR = "Kd";
	static const std::string SPECULAR_POWER = "Ns";

	// parse the materials
	std::string line;
	std::string temp;
	size_t found;
	std::pair<std::string, MaterialTextures> material;
	while(!fin.eof())
	{
		getline(fin, line);

		found = line.find(NEWMTL);
		if(found != std::string::npos)
		{
			temp.assign(line.begin() + NEWMTL.size() + 1, line.end());
			material.first = temp;
			m_Materials[temp] = MaterialTextures();
			continue;
		}

		found = line.find(TEXTURE);
		if(found != std::string::npos)
		{
			temp.assign(line.begin() + TEXTURE.size() + 2, line.end());
			if(material.first.empty())
			{
				m_Error = MTL_PARSE_ERROR;
				return;
			}
			m_Materials[material.first].Diffuse = temp;
		}

		found = line.find(BUMP);
		if(found != std::string::npos)
		{
			temp.assign(line.begin() + BUMP.size() + 2, line.end());
			if(material.first.empty())
			{
				m_Error = MTL_PARSE_ERROR;
				return;
			}
			m_Materials[material.first].NormalMap = temp;
		}

		found = line.find(MASK);
		if(found != std::string::npos)
		{
			temp.assign(line.begin() + MASK.size() + 2, line.end());
			if(material.first.empty())
			{
				m_Error = MTL_PARSE_ERROR;
				return;
			}
			m_Materials[material.first].Mask = temp;
		}
		
		found = line.find(SPECULAR);
		if(found != std::string::npos)
		{
			temp.assign(line.begin() + SPECULAR.size() + 2, line.end());
			if(material.first.empty())
			{
				m_Error = MTL_PARSE_ERROR;
				return;
			}
			m_Materials[material.first].Specular = temp;
		}

		found = line.find(SPECULAR_POWER);
		if (found != std::string::npos)
		{
			temp.assign(line.begin() + SPECULAR_POWER.size() + 2, line.end());
			if (material.first.empty())
			{
				m_Error = MTL_PARSE_ERROR;
				return;
			}
			m_Materials[material.first].SpecularPower = boost::lexical_cast<float>(temp);
		}

		found = line.find(DIFFUSE_COLOR);
		if(found != std::string::npos && found == 1)
		{
			temp.assign(line.begin() + DIFFUSE_COLOR.size() + 2, line.end());
			if(material.first.empty())
			{
				m_Error = MTL_PARSE_ERROR;
				return;
			}
			std::vector<std::string> splitted;
			boost::split(splitted, temp, boost::is_any_of(" "), boost::token_compress_on);

			if(splitted.size() < 3)
			{
				m_Error = MTL_PARSE_ERROR;
				return;
			}

			m_Materials[material.first].DiffuseColor.x = boost::lexical_cast<float>(splitted[0]);
			m_Materials[material.first].DiffuseColor.y = boost::lexical_cast<float>(splitted[1]);
			m_Materials[material.first].DiffuseColor.z = boost::lexical_cast<float>(splitted[2]);
		}
	}

	m_IsOpen = true;
}

bool MtlLoader::IsOpen()
{
	return m_IsOpen;
}

MTL_ERROR MtlLoader::GetLastError()
{
	return m_Error;
}

MtlLoader::MaterialTextures MtlLoader::GetMaterialInfo(const std::string& material)
{
	MaterialsMap::const_iterator it = m_Materials.find(material);
	if(it == m_Materials.end())
	{
		return MaterialTextures();
	}
	
	return it->second;
}

#endif