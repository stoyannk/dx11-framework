// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once
#include "TriangleClipper.h"

// NB: Very Experimental
class SoftwareRasterizer
{
public:
	typedef unsigned long Color;

	enum PixelFormat
	{
		RGBA,
	};
	enum FillTypes
	{
		Solid,
		Wireframe,
	};

	struct PositionColorVertex
	{
		float x;
		float y;
		float z;

		float color[4];
	};
	
public:
	SoftwareRasterizer();

	void SetRenderTarget(void* bytes, int pitch, int cols, int rows, PixelFormat format);
	void SetCullingMode(TriangleClipper::Culling culling);
	TriangleClipper::Culling GetCullingMode() const;
	void SetGlobalMatrices(const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& projection);
	bool DrawIndexedTriangles(const DirectX::XMFLOAT4X4& world, PositionColorVertex* vertices, int* indices, int indicesCount);
		
private:
	struct Edge
	{
		DirectX::XMFLOAT3 Vertex1;
		DirectX::XMFLOAT3 Vertex2;

		int ymin;
		int ymax;
		float oneoverm;
		float x;

		int triagId;

		void CalcProperties();
	};
	
	struct Triangle
	{
		DirectX::XMFLOAT3 Vertices[3];

		float A;
		float B;
		float C;

		Color Color;

		float CurrentZ;

		void InterpolateZ(Edge* e, int y);
		void IncreaseZ();
		void CalcProperties();
	};
	typedef std::vector<SoftwareRasterizer::Triangle*> TrianglesSet;

	void ScanlineFill(Edge* edges, int edgesCount, Triangle* triangles);
	void FillInterval(int line, int startx, int endx, TrianglesSet& AT);

	struct RT
	{
		void* Bytes;
		int Pitch;
		int Cols;
		int Rows;
		PixelFormat Pf;
	} m_RenderTarget;

	struct ClipVertex
	{
		float x;
		float y;
		float z;
		float w;
	};

	DirectX::XMFLOAT4X4 m_ViewProj;
	boost::scoped_array<std::vector<SoftwareRasterizer::Edge*>> m_ActivationTable;
	TriangleClipper::Culling m_CullingMode;
};