// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#include "stdafx.h"

#include "Subset.h"

using namespace DirectX;

Subset::Subset(ID3D11Buffer* ib, size_t indicesCnt, const AABB& bbox)
	: m_IndexBuffer(ib)
	, m_IndicesCnt(indicesCnt)
	, m_AABB(bbox)
{}

Subset::Subset(ID3D11Buffer* ib, size_t indicesCnt, Material material, const AABB& bbox)
	: m_IndexBuffer(ib)
	, m_IndicesCnt(indicesCnt)
	, m_Material(material)
	, m_AABB(bbox)
{}

Subset::~Subset()
{
	SafeRelease(m_IndexBuffer);
}

ID3D11Buffer* Subset::GetIndexBuffer() const
{
	return m_IndexBuffer;
}

size_t Subset::GetIndicesCount() const
{
	return m_IndicesCnt;
}

Material& Subset::GetMaterial()
{
	return m_Material;
}

void* Subset::operator new(size_t size)
{
	return _aligned_malloc(size, 16);
}

void Subset::operator delete(void* data)
{
	_aligned_free(data);
}
