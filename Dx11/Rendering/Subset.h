// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once
#include "Material.h"
#include "BoundingVolumes.h"

struct ID3D11Buffer;

class Subset
{
public:
	Subset(ID3D11Buffer* ib, size_t indicesCnt, const AABB& bbox);
	Subset(ID3D11Buffer* ib, size_t indicesCnt, Material material, const AABB& bbox);

	virtual ~Subset();

	ID3D11Buffer* GetIndexBuffer() const;

	size_t GetIndicesCount() const;

	Material& GetMaterial();
	
	AABB GetAABB();

	void* operator new(size_t size);
	void operator delete(void* data);

private:
	AABB m_AABB;
	ID3D11Buffer* m_IndexBuffer;
	size_t m_IndicesCnt;
	Material m_Material;
};

inline AABB Subset::GetAABB()
{
	return m_AABB;
}

typedef std::shared_ptr<Subset> SubsetPtr;