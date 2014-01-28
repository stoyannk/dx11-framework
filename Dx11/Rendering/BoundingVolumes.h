// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once

struct OOBB
{
	DirectX::XMVECTOR Points[8];
};

struct AABB
{
	DirectX::XMVECTOR Min;
	DirectX::XMVECTOR Max;
};

template<typename VertType>
void ComputeObjectAABB(VertType* vertices, int* indices, size_t indicesCount, OOBB& bbox)
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

	// Compute the OOBB
	bbox.Points[0] = XMLoadFloat4(&min);
	bbox.Points[1] = XMLoadFloat4(&XMFLOAT4(min.x, min.y, max.z, 1));
	bbox.Points[2] = XMLoadFloat4(&XMFLOAT4(min.x, max.y, min.z, 1));
	bbox.Points[3] = XMLoadFloat4(&XMFLOAT4(min.x, max.y, max.z, 1));
	
	bbox.Points[4] = XMLoadFloat4(&XMFLOAT4(max.x, min.y, min.z, 1));
	bbox.Points[5] = XMLoadFloat4(&XMFLOAT4(max.x, min.y, max.z, 1));
	bbox.Points[6] = XMLoadFloat4(&XMFLOAT4(max.x, max.y, min.z, 1));
	bbox.Points[7] = XMLoadFloat4(&max);
}
