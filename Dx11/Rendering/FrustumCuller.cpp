// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#include "stdafx.h"
#include "FrustumCuller.h"
#include "Mesh.h"

using namespace DirectX;

FrustumCuller::FrustumCuller(const XMFLOAT4X4& view, const XMFLOAT4X4& projection)
	: m_View(view)
	, m_Projection(projection)
	, m_ArePlanesDirty(true)
{
	RecalcPlanes();
}

void FrustumCuller::SetView(const XMFLOAT4X4& view)
{
	m_View = view;
	m_ArePlanesDirty = true;
}

void FrustumCuller::SetProjection(const XMFLOAT4X4& proj)
{
	m_Projection = proj;
	m_ArePlanesDirty = true;
}

void FrustumCuller::Cull(const std::vector<Mesh>& meshes, std::vector<SubsetPtr>& outSubsets)
{
	Cull(&meshes[0], meshes.size(), outSubsets);
}

void FrustumCuller::Cull(const Mesh* meshes, unsigned meshCount, std::vector<SubsetPtr>& outSubsets)
{
	if(m_ArePlanesDirty)
	{
		RecalcPlanes();
	}

	std::for_each(meshes, meshes + meshCount, [&] (const Mesh& mesh)
	{
		size_t subsets = mesh.GetSubsetCount();
		for(size_t i = 0; i < subsets; ++i)
		{
			SubsetPtr subset = mesh.GetSubset(i);
			if(IsSubsetVisible(subset))
			{
				outSubsets.push_back(subset);
			}
		}
	});
}

void FrustumCuller::Cull(const EntityVec& entities, EntityToDrawVec& outEntitiesToDraw)
{
	if (m_ArePlanesDirty)
	{
		RecalcPlanes();
	}

	for (auto entity : entities) {
		auto rotAndScale = XMMatrixScaling(entity.Scale, entity.Scale, entity.Scale) * XMMatrixRotationQuaternion(entity.Rotation);
		auto transl = XMLoadFloat3A(&entity.Position);

		std::vector<SubsetPtr> outSubsets;
		size_t subsets = entity.Mesh->GetSubsetCount();
		for (size_t i = 0; i < subsets; ++i)
		{
			SubsetPtr subset = entity.Mesh->GetSubset(i);
			if (IsSubsetVisible(subset, &rotAndScale, &transl))
			{
				outSubsets.push_back(subset);
			}
		}

		if (outSubsets.size()) {
			EntityToDraw toDraw;
			toDraw.Geometry = entity.Mesh.get();
			toDraw.WorldMatrix = rotAndScale * XMMatrixTranslationFromVector(transl);
			toDraw.Subsets = std::move(outSubsets);
			outEntitiesToDraw.emplace_back(toDraw);
		}
	}
}

bool FrustumCuller::IsSubsetVisible(SubsetPtr subset,
	const DirectX::XMMATRIX* tranfs,
	const DirectX::XMVECTOR* translation)
{
	AABB aabb;

	if (tranfs && translation) {
		TransformAABB(subset->GetAABB(), *tranfs, *translation, aabb);
	}
	else
	{
		aabb = subset->GetAABB();
	}

	XMFLOAT4 aabbmin, aabbmax;
	XMStoreFloat4(&aabbmin, aabb.Min);
	XMStoreFloat4(&aabbmax, aabb.Max);

	XMFLOAT4 minExtreme;

	for(unsigned i = 0; i < 6; i++)
	{
		if (m_Planes[i].x <= 0)
		{
			minExtreme.x = aabbmin.x;
		}
		else
		{
			minExtreme.x = aabbmax.x;
		}

		if (m_Planes[i].y <= 0)
		{
			minExtreme.y = aabbmin.y;
		}
		else
		{
			minExtreme.y = aabbmax.y;
		}

		if (m_Planes[i].z <= 0)
		{
			minExtreme.z = aabbmin.z;
		}
		else
		{
			minExtreme.z = aabbmax.z;
		}

		if ((m_Planes[i].x * minExtreme.x + m_Planes[i].y * minExtreme.y + m_Planes[i].z * minExtreme.z + m_Planes[i].w) < 0.f)
		{
			return false;
		}
	}

	return true;
}

void FrustumCuller::CalculateFrustumPlanes(const XMFLOAT4X4& viewMat, const XMFLOAT4X4& projMat, XMFLOAT4 outPlanes[6])
{
	XMMATRIX view = XMLoadFloat4x4(&viewMat);
	XMMATRIX proj = XMLoadFloat4x4(&projMat);
	XMMATRIX viewprojmat = XMMatrixMultiply(view, proj);
	
	XMFLOAT4X4 viewproj;
	XMStoreFloat4x4(&viewproj, viewprojmat);

	// Left clipping plane
	outPlanes[0].x = viewproj._14 + viewproj._11;
	outPlanes[0].y = viewproj._24 + viewproj._21;
	outPlanes[0].z = viewproj._34 + viewproj._31;
	outPlanes[0].w = viewproj._44 + viewproj._41;

	// Right clipping plane
	outPlanes[1].x = viewproj._14 - viewproj._11;
	outPlanes[1].y = viewproj._24 - viewproj._21;
	outPlanes[1].z = viewproj._34 - viewproj._31;
	outPlanes[1].w = viewproj._44 - viewproj._41;

	// Top clipping plane
	outPlanes[2].x = viewproj._14 - viewproj._12;
	outPlanes[2].y = viewproj._24 - viewproj._22;
	outPlanes[2].z = viewproj._34 - viewproj._32;
	outPlanes[2].w = viewproj._44 - viewproj._42;

	// Bottom clipping plane
	outPlanes[3].x = viewproj._14 + viewproj._12;
	outPlanes[3].y = viewproj._24 + viewproj._22;
	outPlanes[3].z = viewproj._34 + viewproj._32;
	outPlanes[3].w = viewproj._44 + viewproj._42;

	// Near clipping plane
	outPlanes[4].x = viewproj._13;
	outPlanes[4].y = viewproj._23;
	outPlanes[4].z = viewproj._33;
	outPlanes[4].w = viewproj._43;

	// Far clipping plane
	outPlanes[5].x = viewproj._14 - viewproj._13;
	outPlanes[5].y = viewproj._24 - viewproj._23;
	outPlanes[5].z = viewproj._34 - viewproj._33;
	outPlanes[5].w = viewproj._44 - viewproj._43;

	// Normalize
	for(unsigned i = 0; i < 6; ++i)
	{
		float norm = sqrt(outPlanes[i].x * outPlanes[i].x
						+ outPlanes[i].y * outPlanes[i].y
						+ outPlanes[i].z * outPlanes[i].z);

		outPlanes[i].x /= norm;
		outPlanes[i].y /= norm;
		outPlanes[i].z /= norm;
		outPlanes[i].w /= norm;
	}
}

void FrustumCuller::RecalcPlanes()
{
	CalculateFrustumPlanes(m_View, m_Projection, m_Planes);
	m_ArePlanesDirty = false;
}

void TransformAABB(const AABB& original,
	const DirectX::XMMATRIX& transform,
	const DirectX::XMVECTOR& translation,
	AABB& result)
{
	using namespace DirectX;

	for (int i = 0; i < 3; ++i) {
		const auto transl = XMVectorGetByIndex(translation, i);
		result.Min = XMVectorSetByIndex(result.Min, transl, i);
		result.Max = XMVectorSetByIndex(result.Max, transl, i);

		for (int j = 0; j < 3; j++) {
			float e = XMVectorGetByIndex(transform.r[j], i) * XMVectorGetByIndex(original.Min, j);
			float f = XMVectorGetByIndex(transform.r[j], i) * XMVectorGetByIndex(original.Max, j);

			auto rmini = XMVectorGetByIndex(result.Min, i);
			auto rmaxi = XMVectorGetByIndex(result.Max, i);
			if (e < f) {
				result.Min = XMVectorSetByIndex(result.Min, rmini + e, i);
				result.Max = XMVectorSetByIndex(result.Max, rmaxi + f, i);
			}
			else
			{
				result.Min = XMVectorSetByIndex(result.Min, rmini + f, i);
				result.Max = XMVectorSetByIndex(result.Max, rmaxi + e, i);
			}
		}
	}
}