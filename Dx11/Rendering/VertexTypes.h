// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once

struct PositionVertex
{
	PositionVertex()
	{}

	PositionVertex(const DirectX::XMFLOAT3& pos)
		: Position(pos)
	{}
	
	DirectX::XMFLOAT3 Position;
};

struct PositionNormalVertex
{
	PositionNormalVertex()
	{}

	PositionNormalVertex(const DirectX::XMFLOAT3& pos,
		const DirectX::XMFLOAT3& normal)
		: Position(pos)
		, Normal(normal)
	{}

	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 Normal;
};

struct PositionColorVertex
{
	PositionColorVertex()
	{}

	PositionColorVertex(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT4& color )
		: Position(pos)
		, Color(color)
	{}
	
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT4 Color;
};

struct PositionTextureVertex
{
	PositionTextureVertex() 
	{}

	PositionTextureVertex(float x, float y, float z, float u, float v)
		: Position(x, y, z)
		, UV(u, v)
	{}

	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT2 UV;
};

struct StandardVertex
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT2 UV;
	DirectX::XMFLOAT3 Tangent;
	DirectX::XMFLOAT3 Bitangent;
};

extern D3D11_INPUT_ELEMENT_DESC PositionVertexLayout[1];
extern D3D11_INPUT_ELEMENT_DESC PositionNormalVertexLayout[2];
extern D3D11_INPUT_ELEMENT_DESC PositionColorVertexLayout[2];
extern D3D11_INPUT_ELEMENT_DESC PositionTextureVertexLayout[2];
extern D3D11_INPUT_ELEMENT_DESC StandardVertexLayout[5];