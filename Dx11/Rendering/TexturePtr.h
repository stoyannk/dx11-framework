// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once

struct ID3D11ShaderResourceView;
struct ID3D11RenderTargetView;

class TextureHolder
{
public:
	TextureHolder()
		: m_ResourceView(nullptr)
		, m_RenderTargetView(nullptr)
	{}

	TextureHolder(ID3D11ShaderResourceView* view)
		: m_ResourceView(view)
		, m_RenderTargetView(nullptr)
	{}

	TextureHolder(ID3D11ShaderResourceView* view, ID3D11RenderTargetView* rtv)
		: m_ResourceView(view)
		, m_RenderTargetView(rtv)
	{}

	ID3D11ShaderResourceView* GetSHRV()
	{
		return m_ResourceView;
	}
	
	const ID3D11ShaderResourceView* GetSHRV() const
	{
		return m_ResourceView;
	}

	ID3D11RenderTargetView* GetRTV()
	{
		return m_RenderTargetView;
	}

	const ID3D11RenderTargetView* GetRTV() const
	{
		return m_RenderTargetView;
	}

	bool IsRenderTarget() const
	{
		return !!m_RenderTargetView;
	}

	bool IsValid() const
	{
		return !!m_ResourceView;
	}

private:
	ID3D11ShaderResourceView* m_ResourceView;
	ID3D11RenderTargetView* m_RenderTargetView;
};

typedef std::shared_ptr<TextureHolder> TexturePtr;
typedef std::weak_ptr<TextureHolder> TextureWeakPtr;

struct TextureReleaser
{
	void operator()(TextureHolder* holder)
	{
		auto srv = holder->GetSHRV();
		SafeRelease(srv);
		auto rtv = holder->GetRTV();
		SafeRelease(rtv);
	}
};
