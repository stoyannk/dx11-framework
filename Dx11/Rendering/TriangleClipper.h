// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once

class TriangleClipper
{
public:
	enum Culling
	{
		CW,
		CCW,
		None
	};

	static int ClipTriangle(DirectX::XMVECTOR vertex[3], DirectX::XMFLOAT4 output[64 * 3], Culling cullingMode);
	static void ClipTriangles(DirectX::XMVECTOR *vertices, int *indices, int indexCount, Culling cullingMode, std::vector<DirectX::XMFLOAT4>& output);
};