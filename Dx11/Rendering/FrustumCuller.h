// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once

#include "Entity.h"

class FrustumCuller
{
public:
	FrustumCuller(const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& projection);

	void SetView(const DirectX::XMFLOAT4X4& view);
	void SetProjection(const DirectX::XMFLOAT4X4& proj);

	void Cull(const std::vector<Mesh>& meshes, std::vector<SubsetPtr>& outSubsets);
	void Cull(const Mesh* meshes, unsigned meshCount, std::vector<SubsetPtr>& outSubsets);

	void Cull(const EntityVec& entities, EntityToDrawVec& outEntitiesToDraw);

	static void CalculateFrustumPlanes(const DirectX::XMFLOAT4X4& viewMat, const DirectX::XMFLOAT4X4& projMat, DirectX::XMFLOAT4 outPlanes[6]);

private:
	void RecalcPlanes();
	bool IsSubsetVisible(SubsetPtr subset,
		const DirectX::XMMATRIX* tranfs = nullptr,
		const DirectX::XMVECTOR* trnaslation = nullptr);

	DirectX::XMFLOAT4X4 m_View;
	DirectX::XMFLOAT4X4 m_Projection;
	
	DirectX::XMFLOAT4 m_Planes[6];
	bool m_ArePlanesDirty;
};