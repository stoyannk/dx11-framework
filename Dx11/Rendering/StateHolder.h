// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once

class StateHolder
{
public:
	enum RSStates
	{
		RST_FrontCW,
		RST_FrontCCW,

		RST_FrontCWWire,
		RST_FrontCCWWire,

		RST_Count
	};
	
	enum BLStates
	{
		BLST_NoWrite,
		BLST_WrOneAddAtoCov,

		BLST_Count
	};

	enum DSStates
	{
		DSST_NoWriteLE,
		DSST_NoWritePass,
		DSST_Count
	};

	bool Initialize(ID3D11Device* device);

	ID3D11RasterizerState* GetRasterState(RSStates state) const
	{
		return m_RasterStates[state].Get();
	}

	ID3D11BlendState* GetBlendState(BLStates state) const
	{
		return m_BlendStates[state].Get();
	}

	ID3D11DepthStencilState* GetDepthState(DSStates state) const
	{
		return m_DepthStates[state].Get();
	}

private:
	ReleaseGuard<ID3D11RasterizerState> m_RasterStates[RST_Count];
	ReleaseGuard<ID3D11BlendState> m_BlendStates[BLST_Count];
	ReleaseGuard<ID3D11DepthStencilState> m_DepthStates[DSST_Count];
};