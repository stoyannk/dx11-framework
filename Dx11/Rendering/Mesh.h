// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once

#include "Subset.h"

class Mesh
{
public:
	Mesh(ID3D11Buffer* vb);

	~Mesh();

	void AddSubset(SubsetPtr subset);

	SubsetPtr GetSubset(size_t id) const;

	size_t GetSubsetCount() const { return m_Subsets.size(); }

	ID3D11Buffer* GetVertexBuffer() const;

	unsigned GetTrianglesCount() const
	{
		return m_TrianglesCount;
	}

private:
	ID3D11Buffer* m_VertexBuffer;
	std::vector<SubsetPtr> m_Subsets;
	unsigned m_TrianglesCount;
};

typedef std::shared_ptr<Mesh> MeshPtr;