// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#include "stdafx.h"
#include "ScreenQuad.h"
#include <Dx11/Rendering/DxRenderer.h>
#include <Dx11/Rendering/ShaderManager.h>
#include <Dx11/Rendering/VertexTypes.h>

using namespace DirectX;

ScreenQuad::ScreenQuad()
	: m_Renderer(nullptr)
	, m_CustomSampler(nullptr)
	, m_RetainDefaultSampler(false)
{}

ScreenQuad::~ScreenQuad()
{}
	
bool ScreenQuad::Initialize(DxRenderer* renderer, const char* shader, const char* vs_entry, const char* ps_entry, D3D11_SAMPLER_DESC* customSamplerDesc)
{
	m_Renderer = renderer;

	ShaderManager shaderManager(m_Renderer->GetDevice());

	ShaderManager::CompilationOutput compilationResult;
	// Create shaders
	if(!shaderManager.CompileShaderDuo(shader
									, vs_entry
									, "vs_4_0"
									, ps_entry
									, "ps_4_0"
									, compilationResult))
	{
		compilationResult.ReleaseAll();
		return false;
	}

	ReleaseGuard<ID3DBlob> vsGuard(compilationResult.vsBlob);
	ReleaseGuard<ID3DBlob> psGuard(compilationResult.psBlob);
	m_VertexShader.Set(compilationResult.vertexShader);
	m_PixelShader.Set(compilationResult.pixelShader);
	
	// Define the input layout
    UINT numElements = ARRAYSIZE(PositionTextureVertexLayout);

	HRESULT hr;
    // Create the input layout
	ID3D11InputLayout* vertexLayout = nullptr;
	hr = m_Renderer->GetDevice()->CreateInputLayout(PositionTextureVertexLayout, numElements, vsGuard.Get()->GetBufferPointer(),
													vsGuard.Get()->GetBufferSize(), &vertexLayout);
	if(FAILED(hr))
	{
		SLOG(Sev_Error, Fac_Rendering, "Unable to create input layout");
		return false;
	}
	m_VertexLayout.Set(vertexLayout);

	PositionTextureVertex vertices[4];
	vertices[0] = PositionTextureVertex(-1, -1, 0, 0, 1);
	vertices[1] = PositionTextureVertex(-1, 1, 0, 0, 0);
	vertices[2] = PositionTextureVertex(1, -1, 0, 1, 1);
	vertices[3] = PositionTextureVertex(1, 1, 0, 1, 0);

	int indices[] =
	{
		0, 1, 2,
		3, 2, 1
	};

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(PositionTextureVertex) * 4;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = vertices;

	ID3D11Buffer* vertexBuffer = nullptr;
    hr = m_Renderer->GetDevice()->CreateBuffer(&bd, &InitData, &vertexBuffer);
    if(FAILED(hr))
	{
		SLOG(Sev_Error, Fac_Rendering, "Unable to create vertex buffer");
		return false;   
	}
	m_VB.Set(vertexBuffer);

	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(int) * 6;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = indices;

	ID3D11Buffer* indexBuffer = nullptr;
    hr = m_Renderer->GetDevice()->CreateBuffer(&bd, &InitData, &indexBuffer);
    if(FAILED(hr))
	{
		SLOG(Sev_Error, Fac_Rendering, "Unable to create index buffer");
		return false;   
	}
	m_IB.Set(indexBuffer);

	// Create samplers
	D3D11_SAMPLER_DESC sampDesc;
	if(!customSamplerDesc)
	{
		::ZeroMemory(&sampDesc, sizeof(sampDesc));
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	}
	ID3D11SamplerState* linearSampler = nullptr;
    hr = m_Renderer->GetDevice()->CreateSamplerState(customSamplerDesc ? customSamplerDesc : &sampDesc, &linearSampler);
	if(FAILED(hr))
	{
		SLOG(Sev_Error, Fac_Rendering, "Unable to create linear sampler state");
		return false;
	}
	m_LinearSampler.Set(linearSampler);

	return true;
}

bool ScreenQuad::DrawDirect(ID3D11DeviceContext* context)
{
	UINT stride = sizeof(PositionTextureVertex);
	UINT offset = 0;

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->IASetVertexBuffers(0, 1, m_VB.GetConstPP(), &stride, &offset);
	context->IASetIndexBuffer(m_IB.Get(), DXGI_FORMAT_R32_UINT, 0);

	context->DrawIndexed(6, 0, 0);

	return true;
}

bool ScreenQuad::Draw(ID3D11ShaderResourceView* texture)
{
	return Draw(&texture, 1);
}

bool ScreenQuad::Draw(ID3D11ShaderResourceView** textures, unsigned texCount)
{
	ID3D11DeviceContext* context = m_Renderer->GetImmediateContext();

	context->IASetInputLayout(m_VertexLayout.Get());

	// Set shaders
	context->VSSetShader(m_VertexShader.Get(), nullptr, 0);
 	context->PSSetShader(m_PixelShader.Get(), nullptr, 0);
	if (texCount)
	{
		context->PSSetShaderResources(0, texCount, textures);
		if (m_CustomSampler)
		{
			context->PSSetSamplers(0, 1, &m_CustomSampler);
			if (m_RetainDefaultSampler)
			{
				context->PSSetSamplers(1, 1, m_LinearSampler.GetConstPP());
			}
		}
		else
		{
			context->PSSetSamplers(0, 1, m_LinearSampler.GetConstPP());
		}
	}
	
	DrawDirect(context);

	if (texCount)
	{
		ID3D11ShaderResourceView* rv[] = { nullptr };
		context->PSSetShaderResources(0, 1, rv);
	}

	return true;
}

void ScreenQuad::SetCustomSampler(ID3D11SamplerState* sampler, bool retainDefaultSampler)
{
	m_CustomSampler = sampler;
	m_RetainDefaultSampler = retainDefaultSampler;
}

