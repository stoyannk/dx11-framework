// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#include "stdafx.h"

#ifndef MINIMAL_SIZE
#include "DxRenderer.h"
#include "RawLoader.h"

#include "VertexTypes.h"
#include "Mesh.h"
#include "MeshSDF.h"
#include "BoundingVolumes.h"
#include "MathConv.h"

using namespace DirectX;

Mesh* RawLoader::Load(DxRenderer* renderer, const std::string& filename, std::string& errors, MeshSDF* sdf)
{
	std::ifstream fin(filename);

	if(!fin.is_open())
	{
		errors += "Unable to open file " + filename;
		return nullptr;
	}

	std::string basePath;
	const size_t slashPos = filename.find_last_of("\\/");
	if(slashPos != filename.npos)
	{
		basePath = filename.substr(0, slashPos) + "\\";
	}

	std::string line;
	getline(fin, line);

	if(line.find(".verts") == line.npos)
	{
		errors += "Invalid mesh file; missing vertices file";
		return nullptr;
	}

	// read vertices
	std::ifstream vertsin(basePath + line, std::ios::binary);
	if(!vertsin.is_open())
	{
		errors += "Unable to open vertices file";
		return nullptr;
	}

	// get length of file
	vertsin.seekg (0, std::ios::end);
	size_t vertslength = size_t(vertsin.tellg());
	vertsin.seekg (0, std::ios::beg);

	boost::scoped_array<char> vertsData(new char[vertslength]);

	vertsin.read(vertsData.get(), vertslength);

	typedef std::vector<std::tuple<boost::shared_array<char>,
		size_t,
		std::string,
		std::string,
		std::string,
		std::string,
		std::string,
		float>> SubsetDataArray;
	SubsetDataArray subsetData;

	std::vector<char> diffuseColor;
	std::vector<char> textureName;
	std::vector<char> normalMapName;
	std::vector<char> maskMapName;
	std::vector<char> specularMapName;
	// read all subsets
	while(!fin.eof())
	{
		getline(fin, line);

		if(line.empty())
			continue;

		if(line.find(".inds") == line.npos)
		{
			errors += "Invalid mesh file; invalid line found: " + line;
			return nullptr;
		}

		// read indices
		std::ifstream indicesin(basePath + line, std::ios::binary);
		if(!indicesin.is_open())
		{
			errors += "Unable to open indices file " + line;
			return nullptr;
		}

		// get length of file
		indicesin.seekg (0, std::ios::end);
		size_t length = size_t(indicesin.tellg());
		indicesin.seekg (0, std::ios::beg);

		//read diffuse color
		unsigned texlen;
		indicesin.read((char*)&texlen, sizeof(unsigned));
		length -= sizeof(unsigned);

		diffuseColor.resize(texlen);
		if(texlen > 0)
		{
			indicesin.read(&diffuseColor[0], texlen);
			length -= texlen;
		}

		// read texture
		indicesin.read((char*)&texlen, sizeof(unsigned));
		length -= sizeof(unsigned);

		textureName.resize(texlen);
		if(texlen > 0)
		{
			indicesin.read(&textureName[0], texlen);
			length -= texlen;
		}

		// read normal map
		indicesin.read((char*)&texlen, sizeof(unsigned));
		length -= sizeof(unsigned);

		normalMapName.resize(texlen);
		if(texlen > 0)
		{
			indicesin.read(&normalMapName[0], texlen);
			length -= texlen;
		}

		//read alpha mask
		indicesin.read((char*)&texlen, sizeof(unsigned));
		length -= sizeof(unsigned);

		maskMapName.resize(texlen);
		if(texlen > 0)
		{
			indicesin.read(&maskMapName[0], texlen);
			length -= texlen;
		}

		//read specular map
		indicesin.read((char*)&texlen, sizeof(unsigned));
		length -= sizeof(unsigned);

		specularMapName.resize(texlen);
		if(texlen > 0)
		{
			indicesin.read(&specularMapName[0], texlen);
			length -= texlen;
		}
		
		//read specular power
		float power;
		indicesin.read((char*)&power, sizeof(float));
		length -= sizeof(float);

		boost::shared_array<char> indssData(new char[length]);

		indicesin.read(indssData.get(), length);

		subsetData.push_back(
			SubsetDataArray::value_type(
			indssData
			, length
			, std::string(diffuseColor.begin(), diffuseColor.end())
			, std::string(textureName.begin(), textureName.end())
			, std::string(normalMapName.begin(), normalMapName.end())
			, std::string(maskMapName.begin(), maskMapName.end())
			, std::string(specularMapName.begin(), specularMapName.end())
			, power
			));
	}

	std::vector<SubsetPtr> subsets;
	std::vector<const unsigned*> indices;
	std::vector<unsigned> indicesSizes;
	AABB meshAABB;
	if (sdf)
	{
		indices.reserve(subsetData.size());
		indicesSizes.reserve(subsetData.size());
	}
	for(SubsetDataArray::const_iterator it = subsetData.begin(); it != subsetData.end(); ++it)
	{
		// create a new subset
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		ID3D11Buffer* indexBuffer = nullptr;
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = std::get<1>(*it);
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA InitData;
		ZeroMemory(&InitData, sizeof(InitData));
		InitData.pSysMem = std::get<0>(*it).get();

		HRESULT hr = renderer->GetDevice()->CreateBuffer(&bd, &InitData, &indexBuffer);
		if(FAILED(hr))
		{
			errors += "Unable to create index buffer";
			return nullptr;
		}
		
		if (sdf)
		{
			indices.push_back((const unsigned*)InitData.pSysMem);
			indicesSizes.push_back(std::get<1>(*it) / sizeof(unsigned));
		}

		// parse the diffuse color
		std::vector<std::string> splitted;
		boost::split(splitted, std::get<2>(*it), boost::is_any_of(" "), boost::token_compress_on);

		if(splitted.size() < 3)
		{
			errors += "Unable to parse diffuse color";
			return nullptr;
		}

		XMFLOAT3 diffuseColor;
		diffuseColor.x = boost::lexical_cast<float>(splitted[0]);
		diffuseColor.y = boost::lexical_cast<float>(splitted[1]);
		diffuseColor.z = boost::lexical_cast<float>(splitted[2]);
		
		TexturePtr texture = renderer->GetTextureManager().Load(std::get<3>(*it), true);
		TexturePtr normals;
		if(std::get<4>(*it).empty())
		{
			errors += "Missing normal map for subset with diffuse texture: " + std::get<3>(*it) + " \n";
		}
		else
		{
			normals = renderer->GetTextureManager().Load(std::get<4>(*it));
		}
		TexturePtr alphaMask;
		if(!std::get<5>(*it).empty())
		{
			alphaMask = renderer->GetTextureManager().Load(std::get<5>(*it));
		}
		TexturePtr specularMap;
		if(!std::get<6>(*it).empty())
		{
			specularMap = renderer->GetTextureManager().Load(std::get<6>(*it));
		}

		AABB bbox;
		ComputeObjectAABB<StandardVertex>(reinterpret_cast<StandardVertex*>(vertsData.get())
									, reinterpret_cast<int*>(std::get<0>(*it).get())
									, std::get<1>(*it) / sizeof(int)
									, bbox);

		Material material;
		material.SetDiffuseColor(diffuseColor);
		material.SetDiffuse(texture);
		material.SetNormalMap(normals);
		material.SetAlphaMask(alphaMask);
		material.SetSpecularMap(specularMap);
		material.SetSpecularPower(std::get<7>(*it));

		SubsetPtr subset(new Subset(indexBuffer, std::get<1>(*it) / sizeof(DWORD), material, bbox));
		subsets.push_back(subset);	
		if (sdf)
		{
			if (it != subsetData.begin())
			{
				meshAABB.Merge(bbox);
			}
			else
			{
				meshAABB = bbox;
			}
		}
	}

	// create vertex buffer
	ID3D11Buffer* vertexBuffer;
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = vertslength;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = vertsData.get();

	HRESULT hr = renderer->GetDevice()->CreateBuffer(&bd, &InitData, &vertexBuffer);
    if(FAILED(hr))
	{
		errors += "Unable to create vertex buffer";
		return nullptr;   
	}

	Mesh* mesh = new Mesh(vertexBuffer);
	std::for_each(subsets.begin(), subsets.end(), std::tr1::bind(&Mesh::AddSubset, mesh, std::tr1::placeholders::_1));

	if (sdf)
	{
		sdf->Populate((const StandardVertex*)vertsData.get(),
			indices.data(),
			indicesSizes.data(),
			subsets.size(),
			toVec3(meshAABB.Min),
			toVec3(meshAABB.Max));
	}

	return mesh;
}

#endif
