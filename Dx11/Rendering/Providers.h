// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once

struct ID3D11Texture2D; 

class IGBufferProvider
{
public:
	virtual ID3D11Texture2D* GetGBufferTexture() = 0;
};

class IVelocityBufferProvider
{
public:
	virtual ID3D11Texture2D* GetVelocityTexture() = 0;
};

class IRenderTargetProvider
{
public:
	virtual ID3D11Texture2D* GetRenderTargetTexture() = 0;
};

class IBuffersProvider
{
public:
	virtual void GetBuffers(std::vector<ID3D11Buffer*>& outBuffers) = 0;
};