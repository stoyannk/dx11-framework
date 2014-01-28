// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once

#include "Subset.h"

class DxRenderer;
class ScreenQuad;

// NB: Very Experimental
class DepthCuller
{
public:
	DepthCuller();
	~DepthCuller();

	bool Initialize(DxRenderer* renderer);

	// TODO:S_ add world matrices
	void ReprojectDepth(const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& projection, const std::vector<SubsetPtr>& inSubsets, std::vector<SubsetPtr>& outSubsets);
	void CopyDepth(const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& projection);

private:
	static const int SurfaceWidth = 256;
	static const int SurfaceHeight = 128;

	DxRenderer* m_Renderer;

	DirectX::XMFLOAT4X4 m_PrevView;
	DirectX::XMFLOAT4X4 m_PrevProjection;
	
	std::unique_ptr<ScreenQuad> m_SQ;

	ReleaseGuard<ID3D11Texture2D> m_LastDepthBuffer;
	ReleaseGuard<ID3D11Texture2D> m_NewDepthBuffer;
	ReleaseGuard<ID3D11Texture2D> m_DownSamplePass1;
	ReleaseGuard<ID3D11RenderTargetView> m_RTDownSample1;
	ReleaseGuard<ID3D11ShaderResourceView> m_SRVDownSample1;
	ReleaseGuard<ID3D11Texture2D> m_DownSamplePass2;
	ReleaseGuard<ID3D11RenderTargetView> m_RTDownSample2;

	ReleaseGuard<ID3D11SamplerState> m_AnisoState;
	float* m_Buffer;

	int m_BBoxIndices[36];
};