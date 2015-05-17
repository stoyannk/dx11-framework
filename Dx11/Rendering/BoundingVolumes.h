// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once

struct AABB
{
	DirectX::XMVECTOR Min;
	DirectX::XMVECTOR Max;

	void Merge(const AABB& other)
	{
		Min = DirectX::XMVectorMin(Min, other.Min);
		Max = DirectX::XMVectorMin(Max, other.Max);
	}

	static AABB Merge(const AABB& b1, const AABB& b2)
	{
		AABB result = b1;
		result.Merge(b2);
		return result;
	}
};

struct OOBB
{
	static OOBB FromAABB(const AABB& aabb)
	{
		using namespace DirectX;
		XMFLOAT4A min;
		XMStoreFloat4A(&min, aabb.Min);
		XMFLOAT4A max;
		XMStoreFloat4A(&max, aabb.Max);

		OOBB bbox;
		bbox.Points[0] = XMLoadFloat4A(&min);
		bbox.Points[1] = XMLoadFloat4A(&XMFLOAT4A(min.x, min.y, max.z, 1));
		bbox.Points[2] = XMLoadFloat4A(&XMFLOAT4A(min.x, max.y, min.z, 1));
		bbox.Points[3] = XMLoadFloat4A(&XMFLOAT4A(min.x, max.y, max.z, 1));

		bbox.Points[4] = XMLoadFloat4A(&XMFLOAT4A(max.x, min.y, min.z, 1));
		bbox.Points[5] = XMLoadFloat4A(&XMFLOAT4A(max.x, min.y, max.z, 1));
		bbox.Points[6] = XMLoadFloat4A(&XMFLOAT4A(max.x, max.y, min.z, 1));
		bbox.Points[7] = XMLoadFloat4A(&max);

		return bbox;
	}

	DirectX::XMVECTOR Points[8];
};

template<typename VertType>
void ComputeObjectAABB(VertType* vertices, int* indices, size_t indicesCount, AABB& bbox)
{
	using namespace DirectX;

	const auto maxFloat = std::numeric_limits<float>::max();

	XMFLOAT4 min(maxFloat, maxFloat, maxFloat, 1);
	XMFLOAT4 max(-maxFloat, -maxFloat, -maxFloat, 1);

	VertType vertex;

	for(unsigned i = 0; i < indicesCount; ++i)
	{
		vertex = vertices[indices[i]];

		min.x = std::min(min.x, vertex.Position.x);
		min.y = std::min(min.y, vertex.Position.y);
		min.z = std::min(min.z, vertex.Position.z);

		max.x = std::max(max.x, vertex.Position.x);
		max.y = std::max(max.y, vertex.Position.y);
		max.z = std::max(max.z, vertex.Position.z);
	}

	bbox.Min = XMLoadFloat4(&min);
	bbox.Max = XMLoadFloat4(&max);
}

void TransformAABB(const AABB& original,
	const DirectX::XMMATRIX& transform,
	const DirectX::XMVECTOR& translation,
	AABB& result);