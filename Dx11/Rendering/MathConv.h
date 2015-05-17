// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once

#include <ThirdParty/glm/glm/glm.hpp>

inline glm::vec3 toVec3(const DirectX::XMFLOAT3& dxVec)
{
	return glm::vec3(dxVec.x, dxVec.y, dxVec.z);
}

inline glm::vec4 toVec4(const DirectX::XMFLOAT4& dxVec)
{
	return glm::vec4(dxVec.x, dxVec.y, dxVec.z, dxVec.w);
}

inline glm::vec3 toVec3(DirectX::FXMVECTOR dxVec)
{
	DirectX::XMFLOAT3 fl3;
	DirectX::XMStoreFloat3(&fl3, dxVec);
	return toVec3(fl3);
}

inline glm::vec4 toVec4(DirectX::FXMVECTOR dxVec)
{
	DirectX::XMFLOAT4 fl4;
	DirectX::XMStoreFloat4(&fl4, dxVec);
	return toVec4(fl4);
}
