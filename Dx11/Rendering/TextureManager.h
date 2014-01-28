// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once

#include "TexturePtr.h"

class TextureManager
{
public:
	TextureManager();
	
	void SetDevice(ID3D11Device* device);

	bool IsAvailable() const;

	TexturePtr Load(const std::string& filename, bool isSRGB = false, bool cube = false);

	TexturePtr LoadTexture2DArray(const std::vector<std::string>& filenames, const std::string& folder, bool isSRGB = false);
	
	std::string GetTextureName(TexturePtr texture) const; // slow

	ID3D11Texture2D* MakeTexture2D(unsigned width, unsigned height, DXGI_FORMAT format, bool shared = false, bool lockable = false);
				   
	TexturePtr MakeAutoTexture2D(unsigned width, unsigned height, DXGI_FORMAT format, bool shared = false);
	
	ID3D11Texture2D* MakeRenderTarget(unsigned width, unsigned height, DXGI_FORMAT format, bool shared = false);

	ID3D11Texture2D* MakeRenderTargetMipped(unsigned width, unsigned height, DXGI_FORMAT format, int mips = 0, int multisampling = 0);

	TexturePtr MakeAutoRenderTarget(unsigned width, unsigned height, DXGI_FORMAT format, bool shared = false);
	
	TexturePtr MakeAutoRenderTargetMipped(unsigned width, unsigned height, DXGI_FORMAT format, int mips = 0, int multisampling = 0);

	ID3D11RenderTargetView* MakeRenderTargetView(ID3D11Resource* renderTarget, DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN);

	ID3D11ShaderResourceView* MakeTextureShaderResourceView(ID3D11Resource* resource, DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN);

	ID3D11SamplerState* MakeSampler(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE mode = D3D11_TEXTURE_ADDRESS_WRAP, unsigned MaxAnisotropy = 0);

	ID3D11DepthStencilView* MakeDepthStencil(unsigned width, unsigned height, DXGI_FORMAT format, DXGI_FORMAT viewFormat);

private:
	ID3D11Device* m_Device;
	std::unordered_map<std::string, TextureWeakPtr> m_Registry;
};