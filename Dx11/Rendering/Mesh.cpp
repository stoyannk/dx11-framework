// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#include "stdafx.h"

#include "Mesh.h"

Mesh::Mesh(ID3D11Buffer* vb)
	: m_VertexBuffer(vb)
{}

Mesh::~Mesh()
{
	SafeRelease(m_VertexBuffer);
}

ID3D11Buffer* Mesh::GetVertexBuffer() const
{
	return m_VertexBuffer;
}

void Mesh::AddSubset(SubsetPtr subset)
{
	m_Subsets.push_back(subset);
}

SubsetPtr Mesh::GetSubset(size_t id) const
{
	assert(id < m_Subsets.size());
	return m_Subsets[id];
}
