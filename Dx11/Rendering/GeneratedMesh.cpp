// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#include "stdafx.h"

#include "GeneratedMesh.h"

#include "ShaderManager.h"
#include "VertexTypes.h"

GeneratedMeshPtr GeneratedMesh::Create(ID3D11Device* device,
	unsigned bufferSize,
	const std::string& generatingFunction)
{
	GeneratedMeshPtr result(new GeneratedMesh(bufferSize, generatingFunction));
	
	ShaderManager shaderManager(device);
	if (!shaderManager.CreateGeneratedBuffer(
		sizeof(PositionNormalVertex), 
		result->GetBufferSize(),
		result->m_GeneratedBuffer.Receive(),
		result->m_GeneratedUAV.Receive(),
		nullptr))
	{
		SLOG(Sev_Error, Fac_Rendering, "Unable to create generated buffer");
		return GeneratedMeshPtr();
	}

	if (!shaderManager.CreateGeneratedBuffer(sizeof(unsigned),
		result->GetBufferSize(),
		result->m_GeneratedIB.Receive(), 
		result->m_GeneratedIBUAV.Receive(),
		nullptr))
	{
		SLOG(Sev_Error, Fac_Rendering, "Unable to create generated index buffer");
		return GeneratedMeshPtr();
	}

	if (!shaderManager.CreateIndexedIndirectBuffer(result->m_IndirectBuffer.Receive(),
		result->m_IndirectUAV.Receive()))
	{
		SLOG(Sev_Error, Fac_Rendering, "Unable to create indirect draw buffer");
		return GeneratedMeshPtr();
	}

	if (!shaderManager.CreateStructuredBuffer(sizeof(unsigned)* 2, 1,
		result->m_CountersBuffer.Receive(),
		result->m_CountersUAV.Receive(),
		nullptr))
	{
		SLOG(Sev_Error, Fac_Rendering, "Unable to create structured counters buffer");
		return GeneratedMeshPtr();
	}

	return result;
}

GeneratedMesh::GeneratedMesh(unsigned buffSize, 
	const std::string& generatingFunction)
	: m_BufferSize(buffSize)
	, m_GeneratingFunction(generatingFunction)
{}

GeneratedMesh::~GeneratedMesh()
{}
