// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once

class DxRenderer;

class ScreenQuad
{
public:
	ScreenQuad();
	~ScreenQuad();
	
	bool Initialize(DxRenderer* renderer, const char* shader, const char* vs_entry = "VS", const char* ps_entry = "PS", D3D11_SAMPLER_DESC* customSamplerDesc = nullptr);

	void SetCustomSampler(ID3D11SamplerState* sampler, bool retainDefaultSampler = false);
	bool Draw(ID3D11ShaderResourceView** textures, unsigned texCount);
	bool Draw(ID3D11ShaderResourceView* texture);

	bool DrawDirect(ID3D11DeviceContext* context);

	bool ReloadShaders();

private:
	ReleaseGuard<ID3D11Buffer> m_VB;
	ReleaseGuard<ID3D11Buffer> m_IB;

	DxRenderer* m_Renderer;
	std::string m_ShaderName;
	std::string m_VSEntry;
	std::string m_PSEntry;

	// Shaders
	ReleaseGuard<ID3D11VertexShader> m_VertexShader;
	ReleaseGuard<ID3D11PixelShader> m_PixelShader;

	// Samplers
	ReleaseGuard<ID3D11SamplerState> m_LinearSampler;
	ID3D11SamplerState* m_CustomSampler;

	// Layout
	ReleaseGuard<ID3D11InputLayout> m_VertexLayout;

	bool m_RetainDefaultSampler;
};