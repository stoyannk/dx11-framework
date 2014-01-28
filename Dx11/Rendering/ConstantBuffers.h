// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once

enum class SystemConsantBuffers
{
	PerFrame = 0,
	PerSubset,

	ConstantBuffersCount
};

struct PerFrameBuffer
{
	DirectX::XMMATRIX View;
	DirectX::XMMATRIX Projection;
	DirectX::XMVECTOR Globals; // backbuffer width; backbuffer height
};
