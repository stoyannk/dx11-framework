// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#include "stdafx.h"
#include "DepthCuller.h"
#include "DxRenderer.h"

#include "ScreenQuad.h"

using namespace DirectX;

//TODO:_ DEBUG
#include "TriangleClipper.h"

DepthCuller::DepthCuller()
	: m_Buffer(nullptr)
{
	XMStoreFloat4x4(&m_PrevView, XMMatrixIdentity());
	XMStoreFloat4x4(&m_PrevProjection, XMMatrixIdentity());

	// Populate the indices
	int i = 0;
	m_BBoxIndices[i++] = 0; m_BBoxIndices[i++] = 6; m_BBoxIndices[i++] = 4;
	m_BBoxIndices[i++] = 0; m_BBoxIndices[i++] = 2; m_BBoxIndices[i++] = 6;
	m_BBoxIndices[i++] = 0; m_BBoxIndices[i++] = 3; m_BBoxIndices[i++] = 2;
	m_BBoxIndices[i++] = 0; m_BBoxIndices[i++] = 1; m_BBoxIndices[i++] = 3;
	m_BBoxIndices[i++] = 2; m_BBoxIndices[i++] = 7; m_BBoxIndices[i++] = 6;
	m_BBoxIndices[i++] = 2; m_BBoxIndices[i++] = 3; m_BBoxIndices[i++] = 7;
	m_BBoxIndices[i++] = 4; m_BBoxIndices[i++] = 6; m_BBoxIndices[i++] = 7;
	m_BBoxIndices[i++] = 4; m_BBoxIndices[i++] = 7; m_BBoxIndices[i++] = 5;
	m_BBoxIndices[i++] = 0; m_BBoxIndices[i++] = 4; m_BBoxIndices[i++] = 5;
	m_BBoxIndices[i++] = 0; m_BBoxIndices[i++] = 5; m_BBoxIndices[i++] = 1;
	m_BBoxIndices[i++] = 1; m_BBoxIndices[i++] = 5; m_BBoxIndices[i++] = 7;
	m_BBoxIndices[i++] = 1; m_BBoxIndices[i++] = 7; m_BBoxIndices[i++] = 3;
}

DepthCuller::~DepthCuller()
{
	delete[] m_Buffer;
}

bool DepthCuller::Initialize(DxRenderer* renderer)
{
	m_Renderer = renderer;

	ID3D11DepthStencilView* depth = m_Renderer->GetBackDepthStencilView();
	D3D11_DEPTH_STENCIL_VIEW_DESC depthDesc;
	depth->GetDesc(&depthDesc);
	// Create a texture to hold the previous depth buffer
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = SurfaceWidth;
	desc.Height = SurfaceHeight;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R32_FLOAT;//depthDesc.Format;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_STAGING;
	desc.BindFlags = 0;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;

	ID3D11Texture2D* depthTex;
	HRESULT hr = m_Renderer->GetDevice()->CreateTexture2D(&desc, nullptr, &depthTex);
	if(FAILED(hr))
	{
		SLOG(Sev_Error, Fac_Rendering, "Unable to create depth buffer staging texture");
		return false;
	}
	m_LastDepthBuffer.Set(depthTex);

	ID3D11Texture2D* newDepthTex;
	hr = m_Renderer->GetDevice()->CreateTexture2D(&desc, nullptr, &newDepthTex);
	if(FAILED(hr))
	{
		SLOG(Sev_Error, Fac_Rendering, "Unable to create new depth buffer staging texture");
		return false;
	}
	m_NewDepthBuffer.Set(newDepthTex);

	m_Buffer = new float[SurfaceWidth * SurfaceHeight];

	m_SQ.reset(new ScreenQuad());

	// Samplers for the downsample
	desc.Width = SurfaceWidth;
	desc.Height = unsigned(m_Renderer->GetBackBufferHeight() / 2);
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	ID3D11Texture2D* downSample1 = nullptr;
	hr = m_Renderer->GetDevice()->CreateTexture2D(&desc, nullptr, &downSample1);
	if(FAILED(hr))
	{
		SLOG(Sev_Error, Fac_Rendering, "Unable to create downsample texture 1");
		return false;
	}
	m_DownSamplePass1.Set(downSample1);
	
	ID3D11RenderTargetView* rtview = nullptr;
	D3D11_RENDER_TARGET_VIEW_DESC rtDesc;
	rtDesc.Format = DXGI_FORMAT_R32_FLOAT;
	rtDesc.Texture2D.MipSlice = 0;
	rtDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	hr = m_Renderer->GetDevice()->CreateRenderTargetView(m_DownSamplePass1.Get(), &rtDesc, &rtview);
	if(FAILED(hr))
	{
		SLOG(Sev_Error, Fac_Rendering, "Unable to create rt view for donsampling texture 1");
		return false;
	}
	m_RTDownSample1.Set(rtview);

	ID3D11ShaderResourceView* srv = nullptr;
	hr = m_Renderer->GetDevice()->CreateShaderResourceView(m_DownSamplePass1.Get(), nullptr, &srv);
	if(FAILED(hr))
	{
		SLOG(Sev_Error, Fac_Rendering, "Unable to create shader resource view for donsampling texture 1");
		return false;
	}
	m_SRVDownSample1.Set(srv);
	
	desc.Height = SurfaceHeight;
	ID3D11Texture2D* downSample2 = nullptr;
	hr = m_Renderer->GetDevice()->CreateTexture2D(&desc, nullptr, &downSample2);
	if(FAILED(hr))
	{
		SLOG(Sev_Error, Fac_Rendering, "Unable to create downsample texture 2");
		return false;
	}
	m_DownSamplePass2.Set(downSample2);
	hr = m_Renderer->GetDevice()->CreateRenderTargetView(m_DownSamplePass2.Get(), &rtDesc, &rtview);
	if(FAILED(hr))
	{
		SLOG(Sev_Error, Fac_Rendering, "Unable to create rt view for donsampling texture 1");
		return false;
	}
	m_RTDownSample2.Set(rtview);
								 
	if(!m_SQ->Initialize(m_Renderer, "..\\Shaders\\ScreenQuad.fx"))
	{
		SLOG(Sev_Error, Fac_Rendering, "Unable to create screen quad");
		return false;
	}

	// Create sampler
	D3D11_SAMPLER_DESC sampDesc;
    ::ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	ID3D11SamplerState* anisoSampler = nullptr;
    hr = m_Renderer->GetDevice()->CreateSamplerState(&sampDesc, &anisoSampler);
	if(FAILED(hr))
	{
		SLOG(Sev_Error, Fac_Rendering, "Unable to create anisotropic sampler state");
		return false;
	}
	m_AnisoState.Set(anisoSampler);

	return true;
}

#define TO_RGBA(r, g, b, a) (((unsigned char)a << 24) | ((unsigned char)r << 16) | ((unsigned char)g << 8) | ((unsigned char)b))
#define READ_DEPTH(color) ((color) >> 24)

void DepthCuller::ReprojectDepth(const XMFLOAT4X4& view, const XMFLOAT4X4& projection, const std::vector<SubsetPtr>& inSubsets, std::vector<SubsetPtr>& outSubsets)
{
	// Project the old depth on the new texture
	D3D11_MAPPED_SUBRESOURCE map;
	D3D11_MAPPED_SUBRESOURCE newMap;

	ID3D11DeviceContext* context = m_Renderer->GetImmediateContext();

	context->Map(m_LastDepthBuffer.Get(), 0, D3D11_MAP_READ, 0, &map);
	context->Map(m_NewDepthBuffer.Get(), 0, D3D11_MAP_WRITE, 0, &newMap);

	const unsigned width = SurfaceWidth;
	const unsigned height = SurfaceHeight;
	XMVECTOR determinant;
	XMMATRIX oldView = XMLoadFloat4x4(&m_PrevView);
	XMMATRIX oldProjection = XMLoadFloat4x4(&m_PrevProjection);
	
	XMMATRIX oldTransform = XMMatrixInverse(&determinant, XMMatrixMultiply(oldView, oldProjection));
	
	XMMATRIX Projection = XMLoadFloat4x4(&projection);
	XMMATRIX View = XMLoadFloat4x4(&view);
	XMMATRIX Transform = XMMatrixMultiply(View, Projection);

	float* data = (float*)map.pData;
	float* newData = (float*)newMap.pData;

	// Clear the new data
	for(unsigned i = 0; i < height; ++i)
	{
		for(unsigned j = 0; j < width; ++j)
		{
			unsigned index = i * width + j;
			m_Buffer[index] = 1.f;
		}
	}

	for(unsigned i = 0; i < height; ++i)
	{
		for(unsigned j = 0; j < width; ++j)
		{
			unsigned index = i * width + j;
			float depth = data[index];
			float x = j / float(width) * 2.f - 1.f;
			float y = -((i / float(height)) * 2.f - 1.f);

			XMVECTOR oldPosSS = XMLoadFloat4(&XMFLOAT4(x, y, depth, 1));
			
			XMVECTOR oldPosWorld = XMVector4Transform(oldPosSS, oldTransform);

			XMVectorScale(oldPosWorld, 1.f / XMVectorGetW(oldPosWorld));

			// TODO:_ REMOVE
			XMFLOAT4 oldPosWorldFloat;
			XMStoreFloat4(&oldPosWorldFloat, oldPosWorld);

			XMVECTOR newPosSS = XMVector4Transform(oldPosWorld, Transform);

			XMFLOAT4 newPos;
			XMStoreFloat4(&newPos, newPosSS);
			newPos.x /= newPos.w;
			newPos.y /= newPos.w;
			newPos.z /= newPos.w;

			unsigned newX = unsigned((newPos.x + 1.f) / 2.f * width);
			unsigned newY = unsigned((1.f - newPos.y) / 2.f * height);

			if(newX >= width || newY >= height)
			{
				continue;
			}

			//assert(newX == i && newY == j);

			m_Buffer[newY * width + newX] = newPos.z;
			//m_Buffer[index] = newPos.z;
		}
	}
	 
	// Dilate
	for(unsigned i = 0; i < height; ++i)
	{
		for(unsigned j = 0; j < width; ++j)
		{
			unsigned index = i * width + j;
	
			if(m_Buffer[index] < 1.f)
			{
				newData[index] = m_Buffer[index];
				continue;
			}
			// Gather all pixels in the kernel
			float max = 0;
			float pixel = 0;
	
			for(int x = -1; x < 2; ++x)
			{
				for(int y = -1; y < 2; ++y)
				{
					if(i + y >= height || i + y < 0 || j + x >= width || j + x < 0)
					{
						continue;
					}
					pixel = m_Buffer[(i + y) * width + j + x];
					if(pixel < 1.f)
					{
						max = std::max(pixel, max);
					}
				}
			}
	
			if(max  == 0)
			{
				max = 1.f;
			}
			newData[index] = max;
			//if(max == 1 && !firstTime) //TODO:_ REMOVE
			//	assert(false);
		}
	}
	
	//TODO:_ Remove
	//static ID3D11Texture2D* testTex = nullptr;
	//if(!testTex)
	//{
	//	ID3D11DepthStencilView* depth = m_Renderer->GetBackDepthStencilView();
	//	D3D11_DEPTH_STENCIL_VIEW_DESC depthDesc;
	//	depth->GetDesc(&depthDesc);
	//	// Create a texture to hold the previous depth buffer
	//	D3D11_TEXTURE2D_DESC desc;
	//	desc.Width = SurfaceWidth;
	//	desc.Height = SurfaceHeight;
	//	desc.MipLevels = 1;
	//	desc.ArraySize = 1;
	//	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//depthDesc.Format;
	//	desc.SampleDesc.Count = 1;
	//	desc.SampleDesc.Quality = 0;
	//	desc.Usage = D3D11_USAGE_STAGING;
	//	desc.BindFlags = 0;
	//	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
	//	desc.MiscFlags = 0;
	//
	//	HRESULT hr = m_Renderer->GetDevice()->CreateTexture2D(&desc, nullptr, &testTex);
	//	if(FAILED(hr))
	//	{
	//		SLOG(Sev_Error, Fac_Rendering, "Unable to create TEST texture");
	//		assert(false);
	//	}
	//}
	//
	//context->Map(testTex, 0, D3D11_MAP_READ, 0, &map);
	//unsigned int* testData = (unsigned int*)map.pData;

	// Clear test data
	//for(int i = 0; i < width * height; ++i)
	//{
	//	testData[i] = 0xFF000000;
	//}
	
	struct Color
	{
		float r;
		float g;
		float b;
		float a;
	};
	static const Color DebugColors[] = 
	{
		{1, 0, 0, 1},
		{1, 1, 0, 1},
		{1, 0, 1, 1},
		{0, 1, 0, 1},
		{0, 1, 1, 1},
		{0, 0, 1, 1},
		{0.5, 0, 0, 1},
		{1, 0.5, 0, 1},
		{0, 0.5, 0.5, 1},
	};
	static const int DebugColorsCount = _countof(DebugColors);

	// Cull
	int debugColorIdx = 0;
	SubsetPtr subset;

	// only draw smth
	//std::vector<SubsetPtr>::const_iterator toDel;
	//for(toDel = inSubsets.begin(); toDel != inSubsets.end(); ++toDel)
	//{
	//	if((*toDel)->GetIndicesCount() == 14592) //15 for the floor // 7833 vase //14592 sin 4ar6af
	//		break;
	//}
	//subset = *toDel;
	//const_cast<std::vector<SubsetPtr>&>(inSubsets).clear();
	//const_cast<std::vector<SubsetPtr>&>(inSubsets).push_back(subset);

	// TODO:_ DEBUG:
	//SoftwareRasterizer rasterizer;
	//rasterizer.SetRenderTarget(testData, map.RowPitch, width, height, SoftwareRasterizer::RGBA);
	//rasterizer.SetGlobalMatrices(view, projection);
	XMFLOAT4X4 world;
	XMStoreFloat4x4(&world, XMMatrixIdentity());
   
	std::vector<XMFLOAT4> clippedTriangles;
	clippedTriangles.reserve(64);
	XMVECTOR BBoxVertices[8];
	for(std::vector<SubsetPtr>::const_iterator it = inSubsets.begin(); it != inSubsets.end(); ++it)
	{
		subset = *it;
		clippedTriangles.clear();
		// Get the bbox and project it
		//AABB bbox = subset->GetAABB();
		OOBB bbox = subset->GetOOBB();

		// Populate the box vertices
		//BBoxVertices[0] = bbox.Min;
		//BBoxVertices[1] = XMVectorSelect(bbox.Min, bbox.Max, XMVectorSelectControl(0, 0, 1, 1)); //XMFLOAT3(bbmin.x, bbmin.y, bbmax.z);
		//BBoxVertices[2] = XMVectorSelect(bbox.Min, bbox.Max, XMVectorSelectControl(0, 1, 0, 1)); //XMFLOAT3(bbmin.x, bbmax.y, bbmin.z);
		//BBoxVertices[3] = XMVectorSelect(bbox.Min, bbox.Max, XMVectorSelectControl(0, 1, 1, 1)); //XMFLOAT3(bbmin.x, bbmax.y, bbmax.z);
		//BBoxVertices[4] = XMVectorSelect(bbox.Min, bbox.Max, XMVectorSelectControl(1, 0, 0, 1)); //XMFLOAT3(bbmax.x, bbmin.y, bbmin.z);
		//BBoxVertices[5] = XMVectorSelect(bbox.Min, bbox.Max, XMVectorSelectControl(1, 0, 1, 1)); //XMFLOAT3(bbmax.x, bbmin.y, bbmax.z);
		//BBoxVertices[6] = XMVectorSelect(bbox.Min, bbox.Max, XMVectorSelectControl(1, 1, 0, 1)); //XMFLOAT3(bbmax.x, bbmax.y, bbmin.z);
		//BBoxVertices[7] = bbox.Max;
		BBoxVertices[0] = bbox.Points[0];
		BBoxVertices[1] = bbox.Points[1];
		BBoxVertices[2] = bbox.Points[2];
		BBoxVertices[3] = bbox.Points[3];
		BBoxVertices[4] = bbox.Points[4];
		BBoxVertices[5] = bbox.Points[5];
		BBoxVertices[6] = bbox.Points[6];
		BBoxVertices[7] = bbox.Points[7];

		//TODO:_ DEBUG: Rasetrize the bboxes
		//debugColorIdx = ++debugColorIdx % DebugColorsCount;

		//SoftwareRasterizer::PositionColorVertex vertices[8];
		//for(int v = 0; v < 8; ++v)
		//{
		//	vertices[v].x = m_BBoxVertices[v].x;
		//	vertices[v].y = m_BBoxVertices[v].y;
		//	vertices[v].z = m_BBoxVertices[v].z;
		//	::memcpy(&vertices[v].color, &DebugColors[debugColorIdx], sizeof(Color));
		//}

		//rasterizer.DrawIndexedTriangles(world, vertices, m_BBoxIndices, 36);

		for(int v = 0; v < 8; ++v)
		{
			BBoxVertices[v] = XMVector4Transform(BBoxVertices[v], Transform);
		}

		TriangleClipper::ClipTriangles(BBoxVertices, m_BBoxIndices, 36, TriangleClipper::None, clippedTriangles);

		if(clippedTriangles.empty())
		{
			continue;
		}

		// Find the closest point and the min/max X, Y
		XMFLOAT3 min(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
		XMFLOAT3 max(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
		for(auto triag = clippedTriangles.begin(); triag != clippedTriangles.end(); ++triag)
		{
			triag->x /= triag->w;
			triag->y /= triag->w;
			triag->z /= triag->w;
			triag->w /= triag->w;
			
			min.x = std::min(triag->x, min.x);
			min.y = std::min(triag->y, min.y);
			min.z = std::min(triag->z, min.z);

			max.x = std::max(triag->x, max.x);
			max.y = std::max(triag->y, max.y);
		}

		int xstart = int(((min.x + 1.f) / 2.f) * (width - 1));
		int xend   = int(((max.x + 1.f) / 2.f) * (width - 1));
		int yend   = int(((1.f - min.y) / 2.f) * (height - 1));
		int ystart = int(((1.f - max.y) / 2.f) * (height - 1));

		assert(xstart >= 0 && xend < width && ystart >= 0 && yend < height);

		// Check all pixels and decide visibility - assume the conservative min depth
		bool iterate = true;
		static const float DEPTH_BIAS = 0.001f;
		for(int y = ystart; y <= yend && iterate; ++y)
		{
			for(int x = xstart; x <= xend && iterate; ++x)
			{
				const int index = y * width + x;
				assert(index < width*height);
				
				//testData[index] = 0xFF0000FF; //TODO:REMOVE

				if(newData[index] + DEPTH_BIAS >= min.z)
				{
					outSubsets.push_back(*it);
					iterate = false;
				}
			}
		}
	}

	//context->Unmap(testTex, 0);	
	//D3DX11SaveTextureToFile(context, testTex, D3DX11_IFF_DDS, "outputTestTex.dds");

	context->Unmap(m_NewDepthBuffer.Get(), 0);
	context->Unmap(m_LastDepthBuffer.Get(), 0);

	//TODO:_ Remove
	// Check the result
	//D3DX11SaveTextureToFile(context, m_LastDepthBuffer.Get(), D3DX11_IFF_DDS, "outputOld.dds");
	//D3DX11SaveTextureToFile(context, m_NewDepthBuffer.Get(), D3DX11_IFF_DDS, "outputNew.dds");
}
 
void DepthCuller::CopyDepth(const XMFLOAT4X4& view, const XMFLOAT4X4& projection)
{
	ID3D11DeviceContext* context = m_Renderer->GetImmediateContext();

	const float width = m_Renderer->GetBackBufferWidth();
	const float height = m_Renderer->GetBackBufferHeight();

	m_SQ->SetCustomSampler(m_AnisoState.Get());

	context->OMSetRenderTargets(1, m_RTDownSample1.GetConstPP(), nullptr);
	D3D11_VIEWPORT vp;
	vp.MinDepth = 0.f;
	vp.MaxDepth = 1.f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width = SurfaceWidth;
	vp.Height = height / 2;
	context->RSSetViewports(1, &vp);

	ID3D11ShaderResourceView* textures[] = {m_Renderer->GetBackDepthStencilShaderView()};
	m_SQ->Draw(textures, 1);

	context->OMSetRenderTargets(1, m_RTDownSample2.GetConstPP(), nullptr);
	vp.Width = SurfaceWidth;
	vp.Height = SurfaceHeight;
	context->RSSetViewports(1, &vp);

	textures[0] = m_SRVDownSample1.Get();
	m_SQ->Draw(textures, 1);

	context->OMSetRenderTargets(0, nullptr, nullptr);

	ID3D11Texture2D* texture = m_LastDepthBuffer.Get();
	context->CopyResource(texture, m_DownSamplePass2.Get());
	
	vp.Width = width;
	vp.Height = height;
	context->RSSetViewports(1, &vp);
	ID3D11DepthStencilView* depth = m_Renderer->GetBackDepthStencilView();
	ID3D11RenderTargetView* rtview = m_Renderer->GetBackBufferView();
	context->OMSetRenderTargets(1, &rtview, depth);
	
	m_PrevView = view;
	m_PrevProjection = projection;
}
