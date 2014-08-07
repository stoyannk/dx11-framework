// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#include "stdafx.h"
#include "StateHolder.h"

bool StateHolder::Initialize(ID3D11Device* device)
{
	// Raster states
	std::function<CD3D11_RASTERIZER_DESC()> rasterGenerators[RST_Count] =
	{
		[]() -> CD3D11_RASTERIZER_DESC {
			CD3D11_RASTERIZER_DESC desc((CD3D11_DEFAULT()));
			desc.CullMode = D3D11_CULL_BACK;
			desc.FillMode = D3D11_FILL_SOLID;
			desc.FrontCounterClockwise = FALSE;
			return desc;
		},
		[]() -> CD3D11_RASTERIZER_DESC {
			CD3D11_RASTERIZER_DESC desc((CD3D11_DEFAULT()));
			desc.CullMode = D3D11_CULL_BACK;
			desc.FillMode = D3D11_FILL_SOLID;
			desc.FrontCounterClockwise = TRUE;
			return desc;
		},
	};

	for (auto gen = 0; gen < RST_Count; ++gen) {
		if (FAILED(device->CreateRasterizerState(&rasterGenerators[gen](), m_RasterStates[gen].Receive())))
		{
			SLOG(Sev_Error, Fac_Rendering, "Unable to create raster state!");
			return false;
		}
	}
	
	// Blend states
	std::function<CD3D11_BLEND_DESC()> blendGenerators[BLST_Count] =
	{
		[]() -> CD3D11_BLEND_DESC {
			CD3D11_BLEND_DESC desc((CD3D11_DEFAULT()));
			for (unsigned i = 0; i < 8; ++i) {
				desc.RenderTarget[i].RenderTargetWriteMask = 0;
			}
			return desc;
		},
		[]() -> CD3D11_BLEND_DESC {
			CD3D11_BLEND_DESC desc((CD3D11_DEFAULT()));
			desc.AlphaToCoverageEnable = true;
			desc.RenderTarget[0].BlendEnable = true;
			return desc;
		},
	};

	for (auto gen = 0; gen < BLST_Count; ++gen) {
		if (FAILED(device->CreateBlendState(&blendGenerators[gen](), m_BlendStates[gen].Receive())))
		{
			SLOG(Sev_Error, Fac_Rendering, "Unable to create blend state!");
			return false;
		}
	}

	// Depth states
	std::function<CD3D11_DEPTH_STENCIL_DESC()> depthGenerators[DSST_Count] =
	{
		[]()->CD3D11_DEPTH_STENCIL_DESC {
			CD3D11_DEPTH_STENCIL_DESC desc((CD3D11_DEFAULT()));
			desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
			return desc;
		},
	};
	for (auto gen = 0; gen < DSST_Count; ++gen) {
		if (FAILED(device->CreateDepthStencilState(&depthGenerators[gen](), m_DepthStates[gen].Receive())))
		{
			SLOG(Sev_Error, Fac_Rendering, "Unable to create depth-stencil state!");
			return false;
		}
	}

	return true;
}
