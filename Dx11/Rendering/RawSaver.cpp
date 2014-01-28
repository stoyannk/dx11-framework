// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#include "stdafx.h"

#include "DxRenderer.h"
#include "RawSaver.h"
#include "Mesh.h"

bool RawSaver::Save(DxRenderer* renderer, Mesh* mesh, const std::string& filename, std::string& errors)
{
	if(!mesh){
		errors += "Null mesh passed";
		return false;
	}
	// get base name for all files
	const size_t dotPos = filename.rfind('.');
	if(dotPos == filename.npos){
		errors += "Invalid filename";
		return false;
	}

	const std::string basepath = filename.substr(0, dotPos);

	std::string basename;
	const size_t slashPos = basepath.find_last_of("\\/");
	if(slashPos != filename.npos){
		basename = basepath.substr(slashPos + 1, basepath.size() - slashPos + 1);
	}
	else {
		basename = basepath;
	}

	// open index file
	std::ofstream fout(filename);
	if(!fout.is_open()){
		errors += "Unable to create index file " + filename;
		return false;
	}

	// write vertices file
	std::string vertsPath = basepath + ".verts";
	std::string vertsName = basename + ".verts";
	std::ofstream verts(vertsPath, std::ios::binary);
	if(!verts.is_open()){
		errors += "Unable to create vertices file";
		return false;
	}

	ID3D11Buffer* vbuffer = mesh->GetVertexBuffer();
	assert(vbuffer && "Missing vertex buffer");

	HRESULT hr;
	// create a staging buffer to copy to
	ID3D11Buffer* vstaging;
	D3D11_BUFFER_DESC vdesc;
	vbuffer->GetDesc(&vdesc);
	vdesc.Usage = D3D11_USAGE_STAGING;
	vdesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	vdesc.BindFlags = 0;
	hr = renderer->GetDevice()->CreateBuffer(&vdesc, nullptr, &vstaging);
	if(FAILED(hr))
	{
		errors += "Unable to create staging vertex buffer";
		return false;
	}
	ReleaseGuard<ID3D11Buffer> stguard(vstaging);

	renderer->GetImmediateContext()->CopyResource(vstaging, vbuffer);

	D3D11_MAPPED_SUBRESOURCE vmap = {0};
	hr = renderer->GetImmediateContext()->Map(stguard.Get(), 0, D3D11_MAP_READ, 0, &vmap);
	if(FAILED(hr))
	{
		errors += "Unable to map vertex buffer";
		return false;
	}

	// write the data
	verts.write((char*)vmap.pData, vmap.RowPitch);

	renderer->GetImmediateContext()->Unmap(vbuffer, 0);

	fout << vertsName << std::endl;

	// write all subsets
	for(size_t i = 0; i < mesh->GetSubsetCount(); ++i)
	{
		std::ostringstream oss;
		oss << "_" << i << ".inds";
		
		std::string subsetPath = basepath + oss.str();
		std::string subsetName = basename + oss.str();
		std::ofstream inds(subsetPath, std::ios::binary);
		if(!inds.is_open()){
			errors += "Unable to create indices file";
			return false;
		}

		ID3D11Buffer* ibuffer = mesh->GetSubset(i)->GetIndexBuffer();
		assert(ibuffer && "Missing index buffer");

		// create a staging buffer to copy to
		ID3D11Buffer* istaging;
		D3D11_BUFFER_DESC idesc;
		ibuffer->GetDesc(&idesc);
		idesc.Usage = D3D11_USAGE_STAGING;
		idesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		idesc.BindFlags = 0;
		hr = renderer->GetDevice()->CreateBuffer(&idesc, nullptr, &istaging);
		if(FAILED(hr))
		{
			errors += "Unable to create staging index buffer";
			return false;
		}
		ReleaseGuard<ID3D11Buffer> stindexguard(istaging);

		renderer->GetImmediateContext()->CopyResource(istaging, ibuffer);

		D3D11_MAPPED_SUBRESOURCE imap = {0};
		hr = renderer->GetImmediateContext()->Map(stindexguard.Get(), 0, D3D11_MAP_READ, 0, &imap);
		if(FAILED(hr))
		{
			errors += "Unable to map index buffer";
			return false;
		}

		// write material
		std::ostringstream diffuseColorStr;
		const Material& material = mesh->GetSubset(i)->GetMaterial();
		// diffuse color
		auto diffuseColor = material.GetDiffuseColor();
		diffuseColorStr << diffuseColor.x << " " << diffuseColor.y << " " << diffuseColor.z;
		auto dcStr = diffuseColorStr.str();
		unsigned strSize = dcStr.size();
		inds.write((char*)&strSize, sizeof(unsigned));
		inds.write((char*)dcStr.c_str(), dcStr.size());

		// diffuse tex
		std::string texName = renderer->GetTextureManager().GetTextureName(material.GetDiffuse());
		strSize = unsigned(texName.size());
		inds.write((char*)&strSize, sizeof(unsigned));
		inds.write((char*)texName.c_str(), texName.size());

		// normal map
		std::string normalMapName = renderer->GetTextureManager().GetTextureName(material.GetNormalMap());
		strSize = unsigned(normalMapName.size());
		inds.write((char*)&strSize, sizeof(unsigned));
		inds.write((char*)normalMapName.c_str(), normalMapName.size());
	
		// alpha mask
		std::string maskMapName = renderer->GetTextureManager().GetTextureName(material.GetAlphaMask());
		strSize = unsigned(maskMapName.size());
		inds.write((char*)&strSize, sizeof(unsigned));
		inds.write((char*)maskMapName.c_str(), maskMapName.size());
		
		// spacular map
		std::string specularMapName = renderer->GetTextureManager().GetTextureName(material.GetSpecularMap());
		strSize = unsigned(specularMapName.size());
		inds.write((char*)&strSize, sizeof(unsigned));
		inds.write((char*)specularMapName.c_str(), specularMapName.size());

		// specular power
		const float power = material.GetSpecularPower();
		inds.write((const char*)&power, sizeof(float));

		// write the data
		inds.write((char*)imap.pData, imap.RowPitch);

		renderer->GetImmediateContext()->Unmap(ibuffer, 0);

		fout << subsetName << std::endl;
	}

	return true;
}
