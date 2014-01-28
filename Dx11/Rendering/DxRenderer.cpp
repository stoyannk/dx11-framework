// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#include "stdafx.h"

#include "DxRenderer.h"
#include "RenderingRoutine.h"
#include "Camera.h"
#include "ShaderManager.h"
#include "ConstantBuffers.h"

using namespace DirectX;

DxRenderer::DxRenderer()
	: m_Device(nullptr)
	, m_ImmediateContext(nullptr)
	, m_SwapChain(nullptr)
	, m_BackBufferView(nullptr)
	, m_SamplesCnt(1)
{}

DxRenderer::~DxRenderer()
{
	if(IsInitialized())
	{
		m_ImmediateContext->ClearState();
	}

	SafeRelease(m_Device);
	SafeRelease(m_ImmediateContext);
	std::for_each(m_DeferredContexts.begin(), m_DeferredContexts.end(), &SafeRelease<ID3D11DeviceContext>);
	SafeRelease(m_SwapChain);
	SafeRelease(m_BackBufferView);
	SafeRelease(m_BackDepthStencilShaderView);
	SafeRelease(m_DepthStencil);
	SafeRelease(m_BackDepthStencilView);
}

bool DxRenderer::IsInitialized() const
{
	return !!m_Device;
}

bool DxRenderer::Initialize(HWND hWnd, bool fullscreen, bool sRGB, int samples)
{
	m_SamplesCnt = samples;

	HRESULT hr = S_OK;

    RECT rc;
    ::GetClientRect(hWnd, &rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    unsigned numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
	unsigned numFeatureLevels = ARRAYSIZE(featureLevels);

    DXGI_SWAP_CHAIN_DESC sd;
    ::ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = sRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = m_SamplesCnt;
    sd.SampleDesc.Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN;
    sd.Windowed = !fullscreen;
	sd.Flags = fullscreen ? DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH : 0;

    for(unsigned driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
    {
        D3D_DRIVER_TYPE driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain(nullptr, driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                                           D3D11_SDK_VERSION, &sd, &m_SwapChain, &m_Device, &m_FeatureLevel, &m_ImmediateContext);
        if(SUCCEEDED(hr))
		{
			m_TexManager.SetDevice(m_Device);
			break;
		}
    }
    if(FAILED(hr))
	{
		SLOG(Sev_Error, Fac_Rendering, "Unable to create device  & swap chain");
		return false;
	}

	std::ostringstream strFlev;
	strFlev << "Created device & swap chain with feature level " << std::hex << m_FeatureLevel;
	SLOG(Sev_Debug, Fac_Rendering, strFlev.str());

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = m_SwapChain->GetBuffer(0, __uuidof( ID3D11Texture2D ), (LPVOID*)&pBackBuffer);
    if(FAILED(hr))
	{
        SLOG(Sev_Error, Fac_Rendering, "Unable get back buffer");
		return false;
	}
    
	hr = m_Device->CreateRenderTargetView(pBackBuffer, nullptr, &m_BackBufferView);
	pBackBuffer->Release();
	if(FAILED(hr))
	{
		SLOG(Sev_Error, Fac_Rendering, "Unable create back buffer view");
        return false;
	}
	
	// Create depth stencil texture
    D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
    descDepth.Width = width;
    descDepth.Height = height;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = /*DXGI_FORMAT_R32_TYPELESS;*/DXGI_FORMAT_R24G8_TYPELESS;
    descDepth.SampleDesc.Count = m_SamplesCnt;
    descDepth.SampleDesc.Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;

    hr = m_Device->CreateTexture2D(&descDepth, nullptr, &m_DepthStencil);
    if(FAILED(hr))
	{
		SLOG(Sev_Error, Fac_Rendering, "Unable create depth stencil texture");
        return false;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDSV.ViewDimension = (m_SamplesCnt == 1) ? D3D11_DSV_DIMENSION_TEXTURE2D : D3D11_DSV_DIMENSION_TEXTURE2DMS;
    descDSV.Texture2D.MipSlice = 0;

	hr = m_Device->CreateDepthStencilView(m_DepthStencil, &descDSV, &m_BackDepthStencilView);
	if(FAILED(hr))
	{
		SLOG(Sev_Error, Fac_Rendering, "Unable create depth stencil view");
        return false;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC dsrvDesc;
	ZeroMemory(&dsrvDesc, sizeof(dsrvDesc));
	dsrvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	dsrvDesc.ViewDimension =(m_SamplesCnt == 1) ? D3D11_SRV_DIMENSION_TEXTURE2D : D3D11_SRV_DIMENSION_TEXTURE2DMS;
	dsrvDesc.Texture2D.MipLevels = 1;
	hr = m_Device->CreateShaderResourceView(m_DepthStencil, &dsrvDesc, &m_BackDepthStencilShaderView);
	if(FAILED(hr))
	{
		SLOG(Sev_Error, Fac_Rendering, "Unable create depth stencil shader resource view");
        return false;
	}

	m_ImmediateContext->OMSetRenderTargets(1, &m_BackBufferView, m_BackDepthStencilView);

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (float)width;
    vp.Height = (float)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    m_ImmediateContext->RSSetViewports(1, &vp);

	m_BackBufferWidth = (float)width;
	m_BackBufferHeight = (float)height;
	
	ShaderManager shaderManager(GetDevice());
	if(!shaderManager.CreateEasyConstantBuffer<PerFrameBuffer>(m_PerFrameConstantBuffer.Receive()))
	{
		SLOG(Sev_Error, Fac_Rendering, "Unable to create per-frame constant buffer");
		return false;
	}

    return true;
}
 
void DxRenderer::AddRoutine(RenderingRoutine* routine)
{
	m_Routines.push_back(routine);
}

void DxRenderer::RemoveRoutine(RenderingRoutine* routine)
{
	m_Routines.erase(std::remove(m_Routines.begin(), m_Routines.end(), routine));
}

void DxRenderer::ClearRoutines()
{
	m_Routines.clear();
}

bool DxRenderer::ReinitRoutineShading()
{
	bool allok = true;
	std::for_each(m_Routines.begin(), m_Routines.end(), [&] (RenderingRoutine* routine)
		{
			if(!routine->ReinitShading())
			{
				allok = false;
				SLOG(Sev_Warning, Fac_Rendering, "Unable to reinitialize a rendering routine");
			}
		});
	
	return allok;
}

ID3D11DeviceContext* DxRenderer::CreateDeferredContext()
{
	ID3D11DeviceContext* context;
	HRESULT hr = m_Device->CreateDeferredContext(0, &context);
	if(FAILED(hr))
	{
		SLOG(Sev_Warning, Fac_Rendering, "Unable to create deferred context");
		return nullptr;
	}
	m_DeferredContexts.push_back(context);

	return context;
}

bool DxRenderer::RemoveDeferredContext(ID3D11DeviceContext* context)
{
	auto it = std::find(m_DeferredContexts.begin(), m_DeferredContexts.end(), context);

	if(it == m_DeferredContexts.end())
	{
		return false;
	}

	m_DeferredContexts.erase(it);
	SafeRelease(context);

	return true;
}

void DxRenderer::SetViewport(ID3D11DeviceContext* context)
{
	// Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = m_BackBufferWidth;
    vp.Height = m_BackBufferHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    context->RSSetViewports(1, &vp);
}

void DxRenderer::SetupFrame(Camera* mainCamera, const XMFLOAT4X4& mainProjection)
{
	//set constant buffers
	PerFrameBuffer pfb;
	pfb.Projection = XMMatrixTranspose(XMLoadFloat4x4(&mainProjection));
	pfb.View = XMMatrixTranspose(XMLoadFloat4x4(&mainCamera->GetViewMatrix()));
	pfb.Globals = XMLoadFloat4(&XMFLOAT4(GetBackBufferWidth(), GetBackBufferHeight(), 0, 0));
	m_ImmediateContext->UpdateSubresource(m_PerFrameConstantBuffer.Get(), 0, nullptr, &pfb, 0, 0);
}

void DxRenderer::PreRender()
{
	// Execute the pre-rendering routines
	std::for_each(m_Routines.begin(), m_Routines.end(), [] (RenderingRoutine* routine) 
		{
			routine->PreRender();
		} );
}

void DxRenderer::Render(float deltaTime)
{
	// Execute the rendering routines
	std::for_each(m_Routines.begin(), m_Routines.end(), [deltaTime] (RenderingRoutine* routine) 
		{
			routine->Render(deltaTime);
		} );
}

void DxRenderer::PostRender()
{
	// Execute the post-rendering routines
	std::for_each(m_Routines.begin(), m_Routines.end(), [] (RenderingRoutine* routine) 
		{
			routine->PostRender();
		} );
}