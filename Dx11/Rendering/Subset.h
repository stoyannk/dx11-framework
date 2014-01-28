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
	Subset(ID3D11Buffer* ib, size_t indicesCnt, OOBB& bbox);
	Subset(ID3D11Buffer* ib, size_t indicesCnt, Material material, OOBB& bbox);

	virtual ~Subset();

	ID3D11Buffer* GetIndexBuffer() const;

	size_t GetIndicesCount() const;

	Material& GetMaterial();
	
	OOBB GetOOBB() const;
	AABB GetAABB();

	void* operator new(size_t size);
	void operator delete(void* data);

private:
	OOBB m_OOBB;
	AABB m_AABB;
	bool m_IsAABBDirty;

	void RecalcAABB();

	ID3D11Buffer* m_IndexBuffer;
	size_t m_IndicesCnt;
	Material m_Material;
};

inline OOBB Subset::GetOOBB() const
{
	return m_OOBB;
}

inline AABB Subset::GetAABB()
{
	if(m_IsAABBDirty)
	{
		RecalcAABB();
	}
	return m_AABB;
}

typedef std::shared_ptr<Subset> SubsetPtr;