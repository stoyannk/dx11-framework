// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#include "stdafx.h"
#include "TextureManager.h"

TextureManager::TextureManager()
	: m_Device(nullptr)
{}

void TextureManager::SetDevice(ID3D11Device* device)
{
	m_Device = device;
}

bool TextureManager::IsAvailable() const
{
	return m_Device != nullptr;
}

DXGI_FORMAT Make_Typeless(DXGI_FORMAT format)
{
    switch( format )
    {
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_R8G8B8A8_UINT:
        case DXGI_FORMAT_R8G8B8A8_SNORM:
        case DXGI_FORMAT_R8G8B8A8_SINT:
            return DXGI_FORMAT_R8G8B8A8_TYPELESS;

        case DXGI_FORMAT_BC1_UNORM_SRGB:
        case DXGI_FORMAT_BC1_UNORM:
            return DXGI_FORMAT_BC1_TYPELESS;
        case DXGI_FORMAT_BC2_UNORM_SRGB:
        case DXGI_FORMAT_BC2_UNORM:
            return DXGI_FORMAT_BC2_TYPELESS;
        case DXGI_FORMAT_BC3_UNORM_SRGB:
        case DXGI_FORMAT_BC3_UNORM:
            return DXGI_FORMAT_BC3_TYPELESS;
    };

    return format;
}

DXGI_FORMAT Make_sRGB(DXGI_FORMAT format)
{
    switch( format )
    {
        case DXGI_FORMAT_R8G8B8A8_TYPELESS:
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_R8G8B8A8_UINT:
        case DXGI_FORMAT_R8G8B8A8_SNORM:
        case DXGI_FORMAT_R8G8B8A8_SINT:
            return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

        case DXGI_FORMAT_BC1_TYPELESS:
        case DXGI_FORMAT_BC1_UNORM:
            return DXGI_FORMAT_BC1_UNORM_SRGB;
        case DXGI_FORMAT_BC2_TYPELESS:
        case DXGI_FORMAT_BC2_UNORM:
            return DXGI_FORMAT_BC2_UNORM_SRGB;
        case DXGI_FORMAT_BC3_TYPELESS:
        case DXGI_FORMAT_BC3_UNORM:
            return DXGI_FORMAT_BC3_UNORM_SRGB;

    };

    return format;
}

TexturePtr TextureManager::Load(const std::string& filename, bool isSRGB, bool cube)
{
	if(!IsAvailable() || filename.empty())
	{
		return TexturePtr();
	}

	auto it = m_Registry.find(filename);
	if(it != m_Registry.end())
	{
		TexturePtr tex = it->second.lock();
		if(tex.get() != nullptr)
		{
			return tex;
		}
		else
		{
			m_Registry.erase(it);
		}
	}
	
	std::wstring wideFile(filename.begin(), filename.end());
	DirectX::ScratchImage image;
	DirectX::TexMetadata metaData;
	HRESULT hr = DirectX::LoadFromDDSFile(wideFile.c_str(), 0, &metaData, image);
	if(FAILED(hr))
	{
		SLOG(Sev_Warning, Fac_Rendering, "Unable to load file for texture ", filename);
		return TexturePtr();
	}

	if (image.GetImageCount() == 1)
	{
		DirectX::ScratchImage mippedImage;
		if (FAILED(DirectX::GenerateMipMaps(image.GetImages()[0], DirectX::TEX_FILTER_DEFAULT, 0, mippedImage)))
		{
			SLOG(Sev_Warning, Fac_Rendering, "Unable to generate mips for texture ", filename);
			return TexturePtr();
		}
		image = std::move(mippedImage);
		metaData = image.GetMetadata();
	}

	if (isSRGB)
	{
		metaData.format = Make_Typeless(metaData.format);;
		image.OverrideFormat(metaData.format);
	}
	
	ID3D11Resource* texture = nullptr;
	hr = DirectX::CreateTexture(m_Device, image.GetImages(), image.GetImageCount(), metaData, &texture);
	if(FAILED(hr))
	{
		SLOG(Sev_Warning, Fac_Rendering, "Unable to load texture ", filename);
		return TexturePtr();
	}

	D3D11_RESOURCE_DIMENSION dimension;
	texture->GetType(&dimension);

	// Create the shader-resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC srDesc;

	if(dimension == D3D11_RESOURCE_DIMENSION_TEXTURE1D)
	{
		ID3D11Texture1D* tex = static_cast<ID3D11Texture1D*>(texture);
		D3D11_TEXTURE1D_DESC desc;
		tex->GetDesc(&desc);
		srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
		srDesc.Format = isSRGB ? Make_sRGB(desc.Format) : desc.Format;
		srDesc.Texture1D.MipLevels = desc.MipLevels;
		srDesc.Texture1D.MostDetailedMip = 0;
	}
	else if(dimension == D3D11_RESOURCE_DIMENSION_TEXTURE2D)
	{
		ID3D11Texture2D* tex = static_cast<ID3D11Texture2D*>(texture);
		D3D11_TEXTURE2D_DESC desc;
		tex->GetDesc(&desc);

		if(metaData.arraySize == 1)
		{
			srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srDesc.Format = isSRGB ? Make_sRGB(desc.Format) : desc.Format;
			srDesc.Texture2D.MipLevels = desc.MipLevels;
			srDesc.Texture2D.MostDetailedMip = 0;
		}
		else if(cube)
		{
			srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
			srDesc.Format = isSRGB ? Make_sRGB(desc.Format) : desc.Format;
			srDesc.TextureCube.MipLevels = desc.MipLevels;
			srDesc.Texture2DArray.MostDetailedMip = 0;
		}
		else
		{
			srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
			srDesc.Format = isSRGB ? Make_sRGB(desc.Format) : desc.Format;
			srDesc.Texture2DArray.MipLevels = desc.MipLevels;
			srDesc.Texture2DArray.MostDetailedMip = 0;
			srDesc.Texture2DArray.ArraySize = metaData.arraySize;
			srDesc.Texture2DArray.FirstArraySlice = 0;
		}
	}
	else if(dimension == D3D11_RESOURCE_DIMENSION_TEXTURE3D)
	{
		ID3D11Texture3D* tex = static_cast<ID3D11Texture3D*>(texture);
		D3D11_TEXTURE3D_DESC desc;
		tex->GetDesc(&desc);
		srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
		srDesc.Format = isSRGB ? Make_sRGB(desc.Format) : desc.Format;
		srDesc.Texture3D.MipLevels = desc.MipLevels;
		srDesc.Texture3D.MostDetailedMip = 0;
	}
	else
	{
		texture->Release();
		if(FAILED(hr))
		{
			SLOG(Sev_Warning, Fac_Rendering, "Loaded unknown resource ", filename);
			return TexturePtr();
		}
	}

	ID3D11ShaderResourceView* view = nullptr;
	hr = m_Device->CreateShaderResourceView(texture, &srDesc, &view);
	if(FAILED(hr))
	{
		texture->Release();
		SLOG(Sev_Error, Fac_Rendering, "Unable to create shader resource-view for resource ", filename);
		return false;
	}

	texture->Release();
	
	TexturePtr result = TexturePtr(new TextureHolder(view), TextureReleaser());
	m_Registry.insert(std::make_pair(filename, TextureWeakPtr(result)));

	return result;
}

TexturePtr TextureManager::LoadTexture2DArray(const std::vector<std::string>& filenames, const std::string& folder, bool isSRGB)
{
	D3D11_TEXTURE2D_DESC desc = {0};
	ReleaseGuard<ID3D11Texture2D> outputTexture;
	ReleaseGuard<ID3D11DeviceContext> context;

	D3D11_MAPPED_SUBRESOURCE mapped;

	m_Device->GetImmediateContext(context.Receive());
	const auto arraySize = filenames.size();
	int texId = 0;
	for(auto filename = filenames.cbegin(); filename != filenames.cend(); ++filename, ++texId)
	{
		auto fullName = folder.size() ? folder + "\\" + *filename : *filename;
		std::wstring wideFile(fullName.begin(), fullName.end());
		DirectX::ScratchImage image;
		DirectX::TexMetadata metaData;
		HRESULT hr = DirectX::LoadFromDDSFile(wideFile.c_str(), 0, &metaData, image);
		if(FAILED(hr))
		{
			SLOG(Sev_Warning, Fac_Rendering, "Unable to file for texture ", fullName);
			return TexturePtr();
		}

		if (image.GetImageCount() == 1)
		{
			DirectX::ScratchImage mippedImage;
			if (FAILED(DirectX::GenerateMipMaps(image.GetImages()[0], DirectX::TEX_FILTER_LINEAR, 0, mippedImage)))
			{
				SLOG(Sev_Warning, Fac_Rendering, "Unable to generate mips for texture ", fullName);
				return TexturePtr();
			}
			image = std::move(mippedImage);
			metaData = image.GetMetadata();
		}

		if (isSRGB)
		{
			metaData.format = Make_Typeless(metaData.format);
			image.OverrideFormat(metaData.format);
		}

		ReleaseGuard<ID3D11Resource> tempResource;
		hr = DirectX::CreateTextureEx(m_Device, image.GetImages(), image.GetImageCount(), metaData,
			D3D11_USAGE_STAGING, 0, D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ, 0, false,
			tempResource.Receive());
		if(FAILED(hr))
		{
			SLOG(Sev_Warning, Fac_Rendering, "Unable to load texture ", fullName);
			return TexturePtr();
		}

		ReleaseGuard<ID3D11Texture2D> texTemp;
		tempResource->QueryInterface( __uuidof( ID3D11Texture2D ), (void**)texTemp.Receive());
        texTemp->GetDesc(&desc);

		if(!outputTexture.Get()) {
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = 0;
			desc.ArraySize = arraySize;
			m_Device->CreateTexture2D(&desc, nullptr, outputTexture.Receive());
		}
        		
		for(auto mip = 0u; mip < desc.MipLevels; ++mip) {
			context->Map(tempResource.Get(), mip, D3D11_MAP_READ, 0, &mapped);
			
			if(!mapped.pData) {
				SLOG(Sev_Error, Fac_Rendering, "Unable to load texture array - mapping texture ", fullName, " failed!");
				return TexturePtr();
			}

			context->UpdateSubresource(outputTexture.Get(), 
				D3D11CalcSubresource(mip, texId, desc.MipLevels),
				nullptr,
				mapped.pData,
				mapped.RowPitch,
				mapped.DepthPitch);

			context->Unmap(tempResource.Get(), mip);
		}
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	::memset(&srvDesc, 0, sizeof(srvDesc));
	srvDesc.Format = isSRGB ? Make_sRGB(desc.Format) : desc.Format;;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Texture2DArray.MipLevels = desc.MipLevels;
	srvDesc.Texture2DArray.ArraySize = arraySize;

	ID3D11ShaderResourceView* view = nullptr;
	HRESULT hr = m_Device->CreateShaderResourceView(outputTexture.Get(), &srvDesc, &view);
	if(FAILED(hr))
	{
		SLOG(Sev_Error, Fac_Rendering, "Unable to create shader resource-view texture array!");
		return TexturePtr();
	}

	return TexturePtr(new TextureHolder(view), TextureReleaser());
}

std::string TextureManager::GetTextureName(TexturePtr texture) const
{
	if(!texture.get()) return "";

	for(auto it = m_Registry.begin(); it != m_Registry.end(); ++it)
	{
		TexturePtr ptr(it->second);
		if(ptr.get() && ptr->GetSHRV() == texture->GetSHRV())
		{
			return it->first;
		}
	}

	return "";
}

ID3D11Texture2D* TextureManager::MakeTexture2D(unsigned width, unsigned height, DXGI_FORMAT format, bool shared, bool lockable)
{
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = format;
	desc.SampleDesc.Count = 1;
	desc.Usage = lockable ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.MiscFlags = shared ? D3D11_RESOURCE_MISC_SHARED : 0;
	if(lockable)
	{
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}

	ID3D11Texture2D* tex = nullptr;
	HRESULT hr = m_Device->CreateTexture2D(&desc, nullptr, &tex);
	if(FAILED(hr))
	{
		SLOG(Sev_Error, Fac_Rendering, "Unable to create render target");
		return nullptr;
	}

	return tex;
}

TexturePtr TextureManager::MakeAutoTexture2D(unsigned width, unsigned height, DXGI_FORMAT format, bool shared)
{
	auto texture = MakeTexture2D(width, height, format, shared);

	if(!texture)
	{
		return TexturePtr();
	}

	auto srv = MakeTextureShaderResourceView(texture);

	texture->Release();

	return TexturePtr(new TextureHolder(srv), TextureReleaser());
}

ID3D11Texture2D* TextureManager::MakeRenderTarget(unsigned width, unsigned height, DXGI_FORMAT format, bool shared)
{
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = format;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	desc.MiscFlags = shared ? D3D11_RESOURCE_MISC_SHARED : 0;

	ID3D11Texture2D* tex = nullptr;
	HRESULT hr = m_Device->CreateTexture2D(&desc, nullptr, &tex);
	if(FAILED(hr))
	{
		SLOG(Sev_Error, Fac_Rendering, "Unable to create render target");
		return nullptr;
	}

	return tex;
}

ID3D11Texture2D* TextureManager::MakeRenderTargetMipped(unsigned width, unsigned height, DXGI_FORMAT format, int mips, int multisampling)
{
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = mips;
	desc.ArraySize = 1;
	desc.Format = format;
	if(!multisampling)
	{
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN;
	}
	else
	{
		UINT count = 0;
		HRESULT hr = m_Device->CheckMultisampleQualityLevels(format, multisampling, &count);
		if(FAILED(hr))
		{
			SLOG(Sev_Error, Fac_Rendering, "Unable to check multisampling qulity for format ", format, " with ", multisampling, " samples");
			return nullptr;
		}
		if(count)
		{
			desc.SampleDesc.Count = multisampling;
			desc.SampleDesc.Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN;
			desc.MipLevels = 1;
		}
		else
		{
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN;
		}
	}
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	ID3D11Texture2D* tex = nullptr;
	HRESULT hr = m_Device->CreateTexture2D(&desc, nullptr, &tex);
	if(FAILED(hr))
	{
		SLOG(Sev_Error, Fac_Rendering, "Unable to create render target");
		return nullptr;
	}

	return tex;
}
 
TexturePtr TextureManager::MakeAutoRenderTarget(unsigned width, unsigned height, DXGI_FORMAT format, bool shared)
{
	auto texture = MakeRenderTarget(width, height, format, shared);

	if(!texture)
	{
		return TexturePtr();
	}

	auto srv = MakeTextureShaderResourceView(texture);

	texture->Release();

	auto rtv = MakeRenderTargetView(texture);

	return TexturePtr(new TextureHolder(srv, rtv), TextureReleaser());
}

TexturePtr TextureManager::MakeAutoRenderTargetMipped(unsigned width, unsigned height, DXGI_FORMAT format, int mips, int multisampling)
{
	auto texture = MakeRenderTargetMipped(width, height, format, mips, multisampling);

	if(!texture)
	{
		return TexturePtr();
	}

	auto srv = MakeTextureShaderResourceView(texture);

	texture->Release();

	auto rtv = MakeRenderTargetView(texture);

	return TexturePtr(new TextureHolder(srv, rtv), TextureReleaser());
}

ID3D11RenderTargetView* TextureManager::MakeRenderTargetView(ID3D11Resource* renderTarget, DXGI_FORMAT format)
{
	D3D11_RESOURCE_DIMENSION dim;
	renderTarget->GetType(&dim);

	if(dim != D3D11_RESOURCE_DIMENSION_TEXTURE2D)
	{
		SLOG(Sev_Error, Fac_Rendering, "Only 2D render targets are supported for views");
		return nullptr;
	}

	ID3D11Texture2D* rt = static_cast<ID3D11Texture2D*>(renderTarget);

	D3D11_TEXTURE2D_DESC desc;
	rt->GetDesc(&desc);

	D3D11_RENDER_TARGET_VIEW_DESC rtDesc;
	rtDesc.Format = (format == DXGI_FORMAT_UNKNOWN ? desc.Format : format);
	rtDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtDesc.Texture2D.MipSlice = 0;

	ID3D11RenderTargetView* rTV = nullptr;
	HRESULT hr = m_Device->CreateRenderTargetView(rt, &rtDesc, &rTV); 
	if(FAILED(hr))
	{
		SLOG(Sev_Error, Fac_Rendering, "Unable to create render target view");
		return nullptr;
	}

	return rTV;
}

ID3D11ShaderResourceView* TextureManager::MakeTextureShaderResourceView(ID3D11Resource* resource, DXGI_FORMAT format)
{
	D3D11_RESOURCE_DIMENSION dim;
	resource->GetType(&dim);

	D3D11_SHADER_RESOURCE_VIEW_DESC srDesc;

	switch(dim)
	{
	case D3D11_RESOURCE_DIMENSION_BUFFER:
		{
			SLOG(Sev_Error, Fac_Rendering, "Only texture shader resource views are supported for automatic creation");
			return nullptr;
		}
		break;
	case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
		{
			ID3D11Texture1D* texture = static_cast<ID3D11Texture1D*>(resource);
			D3D11_TEXTURE1D_DESC desc;
			texture->GetDesc(&desc);

			srDesc.Format = (format == DXGI_FORMAT_UNKNOWN ? desc.Format : format);
			srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
			srDesc.Texture1D.MipLevels = desc.MipLevels;
			srDesc.Texture1D.MostDetailedMip = 0;
		}
		break;
	case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
		{
			ID3D11Texture2D* texture = static_cast<ID3D11Texture2D*>(resource);
			D3D11_TEXTURE2D_DESC desc;
			texture->GetDesc(&desc);

			srDesc.Format = (format == DXGI_FORMAT_UNKNOWN ? desc.Format : format);
			srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srDesc.Texture2D.MipLevels = desc.MipLevels;
			srDesc.Texture2D.MostDetailedMip = 0;
		}
		break;
	case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
		{
			ID3D11Texture3D* texture = static_cast<ID3D11Texture3D*>(resource);
			D3D11_TEXTURE3D_DESC desc;
			texture->GetDesc(&desc);

			srDesc.Format = (format == DXGI_FORMAT_UNKNOWN ? desc.Format : format);
			srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
			srDesc.Texture3D.MipLevels = desc.MipLevels;
			srDesc.Texture3D.MostDetailedMip = 0;
		}
		break;
	}

	ID3D11ShaderResourceView* SRV;
	HRESULT hr = m_Device->CreateShaderResourceView(resource, &srDesc, &SRV);
	if(FAILED(hr))
	{
		SLOG(Sev_Error, Fac_Rendering, "Unable to create shader resource view");
		return nullptr;
	}

	return SRV;
}

ID3D11SamplerState* TextureManager::MakeSampler(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE mode, unsigned MaxAnisotropy)
{
	D3D11_SAMPLER_DESC sampDesc;
    ::ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = filter;
	sampDesc.AddressU = mode;
    sampDesc.AddressV = mode;
    sampDesc.AddressW = mode;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MaxAnisotropy = MaxAnisotropy;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	
	ID3D11SamplerState* sampler = nullptr;
    HRESULT hr = m_Device->CreateSamplerState(&sampDesc, &sampler);
	if(FAILED(hr))
	{
		SLOG(Sev_Error, Fac_Rendering, "Unable to create sampler state");
		return nullptr;
	}

	return sampler;
}

ID3D11DepthStencilView* TextureManager::MakeDepthStencil(unsigned width, unsigned height, DXGI_FORMAT format, DXGI_FORMAT viewFormat)
{
	// Create depth stencil texture
    D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
    descDepth.Width = width;
    descDepth.Height = height;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = format;
    descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;

	ReleaseGuard<ID3D11Texture2D> depthTex;
	HRESULT hr = m_Device->CreateTexture2D(&descDepth, nullptr, depthTex.Receive());
    if(FAILED(hr))
	{
		SLOG(Sev_Error, Fac_Rendering, "Unable create depth stencil texture");
        return nullptr;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
    descDSV.Format = viewFormat;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;

	ID3D11DepthStencilView* result = nullptr;
	hr = m_Device->CreateDepthStencilView(depthTex.Get(), &descDSV, &result);
	if(FAILED(hr))
	{
		SLOG(Sev_Error, Fac_Rendering, "Unable create depth stencil view");
        return nullptr;
	}

	return result;
}
