// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#include "stdafx.h"

#include "Subset.h"

using namespace DirectX;

Subset::Subset(ID3D11Buffer* ib, size_t indicesCnt, OOBB& bbox)
	: m_IndexBuffer(ib)
	, m_IndicesCnt(indicesCnt)
	, m_OOBB(bbox)
	, m_IsAABBDirty(true)
{
	RecalcAABB();
}

Subset::Subset(ID3D11Buffer* ib, size_t indicesCnt, Material material, OOBB& bbox)
	: m_IndexBuffer(ib)
	, m_IndicesCnt(indicesCnt)
	, m_Material(material)
	, m_OOBB(bbox)
	, m_IsAABBDirty(true)
{
	RecalcAABB();
}

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

void Subset::RecalcAABB()
{
	XMFLOAT4 vertex;

	XMFLOAT4 min(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), 1);
	XMFLOAT4 max(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), 1);

	for(unsigned i = 0; i < 8; ++i)
	{
		XMStoreFloat4(&vertex, m_OOBB.Points[i]);

		min.x = std::min(min.x, vertex.x);
		min.y = std::min(min.y, vertex.y);
		min.z = std::min(min.z, vertex.z);

		max.x = std::max(max.x, vertex.x);
		max.y = std::max(max.y, vertex.y);
		max.z = std::max(max.z, vertex.z);
	}

	m_AABB.Min = XMLoadFloat4(&min);
	m_AABB.Max = XMLoadFloat4(&max);

	m_IsAABBDirty = false;
}

void* Subset::operator new(size_t size)
{
	return _aligned_malloc(size, 16);
}

void Subset::operator delete(void* data)
{
	_aligned_free(data);
}
