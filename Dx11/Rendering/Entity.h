// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once

#include "Mesh.h"
#include "GeneratedMesh.h"
#include <Utilities/Aligned.h>
#include <Utilities/StdAllocator.h>

template<typename MeshT>
struct Transformed : private Aligned<16>
{
	Transformed();
	~Transformed();

	DirectX::XMVECTOR Rotation;
	DirectX::XMFLOAT3A Position;
	float Scale;

	MeshT Mesh;
};

typedef Transformed<MeshPtr> Entity;
typedef std::vector<Entity, StdAllocatorAlignedDefault<Entity, 16>> EntityVec;
typedef Transformed<GeneratedMeshPtr> ProceduralEntity;
typedef std::vector<ProceduralEntity, StdAllocatorAlignedDefault<ProceduralEntity, 16>> ProceduralEntityVec;

struct EntityToDraw : private Aligned<16>
{
	DirectX::XMMATRIX WorldMatrix;
	Mesh* Geometry;
	std::vector<SubsetPtr> Subsets;
};
typedef std::vector<EntityToDraw, StdAllocatorAlignedDefault<EntityToDraw, 16>> EntityToDrawVec;

struct ProceduralEntityToDraw : private Aligned<16>
{
	DirectX::XMMATRIX WorldMatrix;
	GeneratedMesh* Geometry;
};
typedef std::vector<ProceduralEntityToDraw, StdAllocatorAlignedDefault<ProceduralEntityToDraw, 16>> ProceduralEntityToDrawVec;
