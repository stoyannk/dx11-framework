// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#include "stdafx.h"

#ifndef MINIMAL_SIZE
#include "DxRenderer.h"
#include "ObjLoader.h"

#include "VertexTypes.h"
#include "Mesh.h"
#include "MtlLoader.h"
#include "Material.h"

using namespace DirectX;

void CalculateTBN(std::vector<StandardVertex>& vertices, int indices[], int ids[3])
{
	StandardVertex& v0 = vertices[indices[ids[0]]];
	StandardVertex& v1 = vertices[indices[ids[1]]];
	StandardVertex& v2 = vertices[indices[ids[2]]];
		
	XMVECTOR A = XMLoadFloat3(&v1.Position) - XMLoadFloat3(&v0.Position);
	XMVECTOR B = XMLoadFloat3(&v2.Position) - XMLoadFloat3(&v0.Position);
 
	XMVECTOR P = XMLoadFloat2(&v1.UV) - XMLoadFloat2(&v0.UV);
    XMVECTOR Q = XMLoadFloat2(&v2.UV) - XMLoadFloat2(&v0.UV);

	XMFLOAT2 fP, fQ;
	XMStoreFloat2(&fP, P); XMStoreFloat2(&fQ, Q);
	float fraction = 1.0f / ( fP.x * fQ.y - fQ.x * fP.y );

	XMFLOAT3 output[3];
	XMStoreFloat3(&output[2], XMVector3Normalize(XMVector3Cross(A, B))); // normal

	XMFLOAT3 fA, fB;
	XMStoreFloat3(&fA, A); XMStoreFloat3(&fB, B);

	output[0] = XMFLOAT3((fQ.y * fA.x - fP.y * fB.x) * fraction, // tangent
						 (fQ.y * fA.y - fP.y * fB.y) * fraction,
						 (fQ.y * fA.z - fP.y * fB.z) * fraction);

	// normalize
	float len = sqrt(output[0].x*output[0].x + output[0].y*output[0].y + output[0].z*output[0].z);
	if(len > 0.0001f) 
	{
		output[0].x /= len; output[0].y /= len; output[0].z /= len;
	}

	output[1] = XMFLOAT3((fP.x * fB.x - fQ.x * fA.x) * fraction, // bitangent
						 (fP.x * fB.y - fQ.x * fA.y) * fraction,
						 (fP.x * fB.z - fQ.x * fA.z) * fraction);

	// normalize
	len = sqrt(output[1].x*output[1].x + output[1].y*output[1].y + output[1].z*output[1].z);
	if(len > 0.0001f) 
	{
		output[1].x /= len; output[1].y /= len; output[1].z /= len;
	}
	XMVECTOR vOutput[3];
	vOutput[0] = XMLoadFloat3(&output[0]);
	vOutput[1] = XMLoadFloat3(&output[1]);
	vOutput[2] = XMLoadFloat3(&output[2]);

	// fix broken tbn due to opposite uv
	if(XMVectorGetX(XMVector3Dot(vOutput[2], XMVector3Cross(vOutput[0], vOutput[1]))) < 0)
	{
		vOutput[0] = XMVectorNegate(vOutput[0]);
	} 

	// Some simple aliases
    float NdotT = XMVectorGetX(XMVector3Dot( vOutput[2], vOutput[0] ));
    float NdotB = XMVectorGetX(XMVector3Dot( vOutput[2], vOutput[1] ));
    float TdotB = XMVectorGetX(XMVector3Dot( vOutput[0], vOutput[1] ));
 
    // Apply Gram-Schmidt orthogonalization
    vOutput[0] = vOutput[0] - NdotT * vOutput[2];
    vOutput[1] = vOutput[1] - NdotB * vOutput[2] - TdotB * vOutput[0];

	XMStoreFloat3(&v0.Normal, vOutput[2]);
	XMStoreFloat3(&v0.Tangent, vOutput[0]);
	XMStoreFloat3(&v0.Bitangent, vOutput[1]);
}

Mesh* ObjLoader::Load(DxRenderer* renderer, const std::string& filename, std::string& errors)
{
	std::ifstream fin(filename);

	if(!fin.is_open())
	{
		errors += "Unable to open file " + filename;
		return nullptr;
	}

	static const std::string MTLLIB = "mtllib";
	static const std::string USEMTL = "usemtl";

	// Read all data for a subset
	std::vector<XMFLOAT3> vertices;
	std::vector<XMFLOAT3> normals;
	std::vector<XMFLOAT2> uvs;

	std::vector<StandardVertex> readyVertices;
	std::vector<int> readyIndices;
	typedef	std::tr1::tuple<int, int, int> UniqueVertex;
	typedef std::map<UniqueVertex, int> AvailableVerticesMap;
	AvailableVerticesMap availVertices;

	std::vector<SubsetPtr> subsets;

	std::string line;
	std::vector<std::string> tokens;
	std::unique_ptr<MtlLoader> mtllib;
	size_t found;
	std::string temp;
	MtlLoader::MaterialTextures currentTextures;

	std::string pathPrefix;
	found = filename.find_last_of("\\");
	if(found != std::string::npos)
	{
		pathPrefix.assign(filename.begin(), filename.begin() + found);
		pathPrefix += "\\";
	}

	auto finalizeSubset = [&]() -> bool
	{
		if(readyIndices.empty())
			return true;
		
		// create a new subset
	    D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		ID3D11Buffer* indexBuffer = nullptr;
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(DWORD) * readyIndices.size();
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA InitData;
		ZeroMemory(&InitData, sizeof(InitData));
		InitData.pSysMem = &readyIndices[0];

		HRESULT hr = renderer->GetDevice()->CreateBuffer(&bd, &InitData, &indexBuffer);
		if(FAILED(hr))
		{
			errors += "Unable to create index buffer";
			return false;
		}

		AABB bbox;
		ComputeObjectAABB<StandardVertex>(&readyVertices[0]
									, reinterpret_cast<int*>(&readyIndices[0])
									, readyIndices.size()
									, bbox);
		
		Material material;
		material.SetDiffuseColor(currentTextures.DiffuseColor);
		material.SetDiffuse(renderer->GetTextureManager().Load(pathPrefix + currentTextures.Diffuse, true));
		material.SetNormalMap(renderer->GetTextureManager().Load(pathPrefix + currentTextures.NormalMap));
		material.SetAlphaMask(renderer->GetTextureManager().Load(pathPrefix + currentTextures.Mask));
		material.SetSpecularMap(renderer->GetTextureManager().Load(pathPrefix + currentTextures.Specular));
		material.SetSpecularPower(currentTextures.SpecularPower);

		SubsetPtr subset(new Subset(indexBuffer, readyIndices.size(), material, bbox));
		subsets.push_back(subset);

		readyIndices.clear();

		return true;
	};

	// Line parsers:
	auto parseVertex = [&]() -> bool
	{
		if(!finalizeSubset())
			return false;

		tokens.clear();
		boost::algorithm::split(tokens, line, boost::algorithm::is_any_of(" "), boost::algorithm::token_compress_on);
		
		if(tokens.size() != 4)
		{
			errors += "Invalid vertex-line found: " + line;
			return false;
		}

		XMFLOAT3 pos;
		try
		{
			pos.x = boost::lexical_cast<float>(tokens[1]);
			pos.y = boost::lexical_cast<float>(tokens[2]);
			pos.z = boost::lexical_cast<float>(tokens[3]);
		}
		catch(boost::bad_lexical_cast& e)
		{
			errors += "Invalid vertex-line found: " + line + "; ->" + e.what();
			return false;
		}
		vertices.push_back(pos);

		return true;
	};
	auto parseNormal = [&]() -> bool
	{
		if(!finalizeSubset())
			return false;

		tokens.clear();
		boost::algorithm::split(tokens, line, boost::algorithm::is_any_of(" "), boost::algorithm::token_compress_on);
		
		if(tokens.size() != 4)
		{
			errors += "Invalid normal-line found: " + line;
			return false;
		}

		XMFLOAT3 normal;
		try
		{
			normal.x = boost::lexical_cast<float>(tokens[1]);
			normal.y = boost::lexical_cast<float>(tokens[2]);
			normal.z = boost::lexical_cast<float>(tokens[3]);
		}
		catch(boost::bad_lexical_cast& e)
		{
			errors += "Invalid normal-line found: " + line + "; ->" + e.what();
			return false;
		}
		normals.push_back(normal);

		return true;
	};
	auto parseUV = [&]() -> bool
	{
		if(!finalizeSubset())
			return false;
		
		tokens.clear();
		boost::algorithm::split(tokens, line, boost::algorithm::is_any_of(" "), boost::algorithm::token_compress_on);
		
		if(tokens.size() < 3)
		{
			errors += "Invalid uv-line found: " + line;
			return false;
		}

		XMFLOAT2 uv;
		try
		{
			uv.x = boost::lexical_cast<float>(tokens[1]);
			uv.y = 1.f - boost::lexical_cast<float>(tokens[2]);
		}
		catch(boost::bad_lexical_cast& e)
		{
			errors += "Invalid uv-line found: " + line + "; ->" + e.what();
			return false;
		}
		uvs.push_back(uv);

		return true;
	};

	auto parseFace = [&]() -> bool
	{
		tokens.clear();
		boost::algorithm::split(tokens, line, boost::algorithm::is_any_of(" "), boost::algorithm::token_compress_on);
		
		if(tokens.size() < 4)
		{
			errors += "Invalid face-line found: " + line;
			return false;
		}
		
		std::vector<int> indicesForThisFace;
		indicesForThisFace.reserve(3);
		std::vector<std::string> faceVals;
		for(size_t i = 1; i < tokens.size(); ++i)
		{
			const std::string& face = tokens[i];
			if(face.empty())
				continue;

			faceVals.clear();
			boost::algorithm::split(faceVals, face, boost::algorithm::is_any_of("/"), boost::algorithm::token_compress_off);

			if(faceVals.size() != 3)
			{
				errors += "Invalid face-line found: " + face + "; not enough tokens";
				return false;
			}

			int idPos = -1, idTex = -1, idNorm = -1;
			try
			{
				idPos = boost::lexical_cast<int>(faceVals[0]);
				if(faceVals[1].size())
				{
					idTex = boost::lexical_cast<int>(faceVals[1]);
				}
				idNorm = boost::lexical_cast<int>(faceVals[2]);

				// c-style indices
				--idPos; --idTex; --idNorm;
			}
			catch(boost::bad_lexical_cast& e)
			{
				errors += "Invalid face-line found: " + face + "; ->" + e.what();
				return false;
			}

			// check for existance
			int vertexIndex = -1;
			auto ids = std::tr1::make_tuple(idPos, idTex, idNorm);
			auto avit = availVertices.find(ids);
			if(avit != availVertices.end()) {
				vertexIndex = avit->second;
			}
			else {
				StandardVertex v;
				v.Position = vertices[idPos];
				if(idTex >= 0) {
					v.UV = uvs[idTex];
				}
				else {
					v.UV = XMFLOAT2(0, 0);
				}
				v.Normal = normals[idNorm];
				readyVertices.push_back(v);
				vertexIndex = readyVertices.size() - 1;
				availVertices.insert(std::make_pair(std::tr1::make_tuple(idPos, idTex, idNorm), vertexIndex));
			}

			if(i < 4)
			{
				readyIndices.push_back(vertexIndex);
				indicesForThisFace.push_back(vertexIndex);
			}
			// it is a quad face - we need to create a full triag for the last index
			else
			{
				readyIndices.push_back(vertexIndex);
				readyIndices.push_back(indicesForThisFace[0]);
				readyIndices.push_back(indicesForThisFace[2]);

				// Calculate TBN for this additional vertex
				int indices[] = {vertexIndex, indicesForThisFace[0], indicesForThisFace[2]};
				int inds[] = {0, 1, 2};
				CalculateTBN(readyVertices, indices, inds);
			}
		}

		int indices[] = {2, 0, 1};
		// Compute tangent and bitangent
		for(int i = 0; i < 3; ++i)
		{
			// Swap array
			int first = indices[0]; indices[0] = indices[1]; indices[1] = indices[2]; indices[2] = first;
			CalculateTBN(readyVertices, &indicesForThisFace[0], indices);			
		}

		return true;
	};

	while(!fin.eof())
	{
		getline(fin, line);

		if(line.empty() || line.size() < 2)
			continue;

		switch(line[0])
		{
		case ' ':
		case '#':
		case 's':
			continue;

		case 'm':
			found = line.find(MTLLIB);
			if(found != std::string::npos)
			{
				temp.assign(line.begin() + MTLLIB.size() + 1, line.end());
				mtllib.reset(new MtlLoader(pathPrefix + temp));
				if(!mtllib->IsOpen())
				{
					errors += "Unable to read mtl library: " + mtllib->GetLastError();
					return nullptr;
				}
			}
			break;
		case 'u':
			found = line.find(USEMTL);
			if(found != std::string::npos)
			{
				// finalize because there are subsets with different materials (!)
				if(!finalizeSubset())
					return nullptr;

				if(mtllib.get() == nullptr)
				{
					errors += "Trying to use a material while there is no material library loaded";
					return nullptr;
				}
				temp.assign(line.begin() + USEMTL.size() + 1, line.end());
				currentTextures = mtllib->GetMaterialInfo(temp);
			}
			break;
		case 'v':
			if(line[1] == 'n') {
				if(!parseNormal())
					return nullptr;
			}
			else if(line[1] == 't') {
				if(!parseUV())
					return nullptr;
			}
			else {
				if(!parseVertex())
					return nullptr;
			}
			break;

		case 'f':
			if(!parseFace())
				return nullptr;
			break;

		case 'g':
			if(!finalizeSubset())
				return nullptr;
			break;

		default:
			errors += "Unkown token found: " + line;
			return nullptr;
		}
	}

	// if there is a final subset
	if(!finalizeSubset())
		return nullptr;

	// create the vertex buffer for all the subsets
	ID3D11Buffer* vertexBuffer;
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(StandardVertex) * readyVertices.size();
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = &readyVertices[0];

	HRESULT hr = renderer->GetDevice()->CreateBuffer(&bd, &InitData, &vertexBuffer);
    if(FAILED(hr))
	{
		errors += "Unable to create vertex buffer";
		return nullptr;   
	}

	Mesh* mesh = new Mesh(vertexBuffer);
	std::for_each(subsets.begin(), subsets.end(), std::tr1::bind(&Mesh::AddSubset, mesh, std::tr1::placeholders::_1));

	return mesh;
}

#endif