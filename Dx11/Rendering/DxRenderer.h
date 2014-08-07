// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once

#include <d3d11.h>
#include "Renderer.h"
#include "TextureManager.h"
#include "StateHolder.h"

class RenderingRoutine;

class DxRenderer : public Renderer
{
public:
	DxRenderer();
	virtual ~DxRenderer();

	bool IsInitialized() const;

	virtual bool Initialize(HWND hWnd, bool fullscreen, bool sRGB = true, int samples = 1);

	virtual void SetupFrame(Camera* mainCamera, const DirectX::XMFLOAT4X4& mainProjection);

	virtual void PreRender();

	virtual void Render(float deltaTime);

	virtual void PostRender();

	virtual void AddRoutine(RenderingRoutine* routine);

	virtual void RemoveRoutine(RenderingRoutine* routine);

	virtual void ClearRoutines();

	virtual bool ReinitRoutineShading();

	D3D_FEATURE_LEVEL GetFeatureLevel() const
	{
		return m_FeatureLevel;
	}

	int SamplesCount() const
	{
		return m_SamplesCnt;
	}

public:
	ID3D11Device* GetDevice() const
	{
		return m_Device;
	}

	ID3D11DeviceContext* GetImmediateContext() const
	{
		return m_ImmediateContext;
	}

	IDXGISwapChain* GetSwapChain() const
	{
		return m_SwapChain;
	}

	ID3D11RenderTargetView* GetBackBufferView() const
	{
		return m_BackBufferView;
	}

	ID3D11DepthStencilView* GetBackDepthStencilView() const
	{
		return m_BackDepthStencilView;
	}

	ID3D11ShaderResourceView* GetBackDepthStencilShaderView() const
	{
		return m_BackDepthStencilShaderView;
	}

	ID3D11Buffer* GetPerFrameConstantBuffer() const
	{
		return m_PerFrameConstantBuffer.Get();
	}
	
	float GetBackBufferWidth() const
	{
		return m_BackBufferWidth;
	}

	float GetBackBufferHeight() const
	{
		return m_BackBufferHeight;
	}

	TextureManager& GetTextureManager()
	{
		return m_TexManager;
	}

	const StateHolder& GetStateHolder() const
	{
		return m_StateHolder;
	}

	ID3D11DeviceContext* CreateDeferredContext();
	bool RemoveDeferredContext(ID3D11DeviceContext* context);
	const std::vector<ID3D11DeviceContext*>& GetDeferredContexts()
	{
		return m_DeferredContexts;
	}

	void SetViewport(ID3D11DeviceContext* context);

private:
	ID3D11Device* m_Device;
	ID3D11DeviceContext* m_ImmediateContext;
	std::vector<ID3D11DeviceContext*> m_DeferredContexts;
	IDXGISwapChain* m_SwapChain;
	ID3D11RenderTargetView* m_BackBufferView;
	ID3D11Texture2D* m_DepthStencil;
	ID3D11DepthStencilView* m_BackDepthStencilView;
	ID3D11ShaderResourceView* m_BackDepthStencilShaderView;
	D3D_FEATURE_LEVEL m_FeatureLevel;
	TextureManager m_TexManager;
	
	ReleaseGuard<ID3D11Buffer> m_PerFrameConstantBuffer;

	StateHolder m_StateHolder;

	float m_BackBufferWidth;
	float m_BackBufferHeight;

	int m_SamplesCnt;

private:
	typedef std::vector<RenderingRoutine*> Routines;
	Routines m_Routines;
};