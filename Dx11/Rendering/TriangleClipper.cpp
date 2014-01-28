// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#include "stdafx.h"
#include "TriangleClipper.h"

using namespace DirectX;

enum ClipPlanes
{
	XPositive = 0,
	XNegative,
	YPositive,
	YNegative,
	ZPositive,
	ZNegative,

	Planes_Count
};

typedef float (XMFLOAT4::*Field);

inline float IntersectLineClipPlane(const XMFLOAT4& A, const XMFLOAT4& B, Field coordinate, bool IsPositive)
{
	if(IsPositive)
	{
		return (A.w - A.*coordinate) / (B.*coordinate - A.*coordinate - B.w + A.w);
	}
	else
	{
		return (-(A.*coordinate) - A.w) / (B.*coordinate - A.*coordinate + B.w - A.w);
	}
}

int ClipTrianglesWithPlane(XMFLOAT4* vertices, int vertsCount, XMFLOAT4* outputVertices, ClipPlanes plane)
{
	XMFLOAT4 in[3];
	XMFLOAT4 out[3];
	int count = 0;
	int inCount = 0;
	int outCount = 0;
	XMFLOAT4* vertex = nullptr;

	Field coordinate;
	bool IsPositive = true;

	switch(plane)
	{
	case XPositive:
		coordinate = &XMFLOAT4::x;
		IsPositive = true;
		break;
	case XNegative:
		coordinate = &XMFLOAT4::x;
		IsPositive = false;
		break;
	case YPositive:
		coordinate = &XMFLOAT4::y;
		IsPositive = true;
		break;
	case YNegative:
		coordinate = &XMFLOAT4::y;
		IsPositive = false;
		break;
	case ZPositive:
		coordinate = &XMFLOAT4::z;
		IsPositive = true;
		break;
	case ZNegative:
		coordinate = &XMFLOAT4::z;
		IsPositive = false;
		break;
	};

	float t;
	for(int i = 0; i < vertsCount; i += 3)
	{
		inCount = outCount = 0;

		for(int vert = 0; vert < 3; ++vert)
		{
			vertex = vertices + i + vert;
			if((IsPositive && vertex->*coordinate > vertex->w)
			|| (!IsPositive && -(vertex->*coordinate) > vertex->w))
			{
				out[outCount++] = *vertex;
			}
			else
			{
				in[inCount++] = *vertex;
			}
		}

		// All in
		if(inCount == 3)
		{
			::memcpy(outputVertices + count, in, sizeof(XMFLOAT4) * 3);
			count += 3;
		}
		// One out
		else if (inCount == 2)
		{
			// Create the new 4 vertices
			//t = (in[0].*coordinate - in[0].w) / (out[0].w - in[0].w - out[0].*coordinate + in[0].*coordinate);
			t = IntersectLineClipPlane(in[0], out[0], coordinate, IsPositive);

			XMFLOAT4 newVert1;
			newVert1.x = in[0].x + t * (out[0].x - in[0].x);
			newVert1.y = in[0].y + t * (out[0].y - in[0].y);
			newVert1.z = in[0].z + t * (out[0].z - in[0].z);
			newVert1.w = in[0].w + t * (out[0].w - in[0].w);

			//t = (in[1].*coordinate - in[1].w) / (out[0].w - in[1].w - out[0].*coordinate + in[1].*coordinate);
			t = IntersectLineClipPlane(in[1], out[0], coordinate, IsPositive);

			XMFLOAT4 newVert2;
			newVert2.x = in[1].x + t * (out[0].x - in[1].x);
			newVert2.y = in[1].y + t * (out[0].y - in[1].y);
			newVert2.z = in[1].z + t * (out[0].z - in[1].z);
			newVert2.w = in[1].w + t * (out[0].w - in[1].w);

			outputVertices[count++] = in[0];
			outputVertices[count++] = newVert1;
			outputVertices[count++] = in[1];

			outputVertices[count++] = newVert2;
			outputVertices[count++] = in[1];
			outputVertices[count++] = newVert1;
		}
		// Two out
		else if(inCount == 1)
		{
			//t = (in[0].*coordinate - in[0].w) / (out[0].w - in[0].w - out[0].*coordinate + in[0].*coordinate);
			t = IntersectLineClipPlane(in[0], out[0], coordinate, IsPositive);

			XMFLOAT4 newVert1;
			newVert1.x = in[0].x + t * (out[0].x - in[0].x);
			newVert1.y = in[0].y + t * (out[0].y - in[0].y);
			newVert1.z = in[0].z + t * (out[0].z - in[0].z);
			newVert1.w = in[0].w + t * (out[0].w - in[0].w);

			//t = (in[0].*coordinate - in[0].w) / (out[1].w - in[0].w - out[1].*coordinate + in[0].*coordinate);
			t = IntersectLineClipPlane(in[0], out[1], coordinate, IsPositive);

			XMFLOAT4 newVert2;
			newVert2.x = in[0].x + t * (out[1].x - in[0].x);
			newVert2.y = in[0].y + t * (out[1].y - in[0].y);
			newVert2.z = in[0].z + t * (out[1].z - in[0].z);
			newVert2.w = in[0].w + t * (out[1].w - in[0].w);

			outputVertices[count++] = in[0];
			outputVertices[count++] = newVert1;
			outputVertices[count++] = newVert2;
		}
		// All out
		else
		{}
	}
	return count;
}

int TriangleClipper::ClipTriangle(XMVECTOR vertex[3], XMFLOAT4 output[64 * 3], Culling cullingMode) // 64 is 2^6 - the max possible outcome 
{
	int count = 0;
	XMFLOAT4 temp[64 * 3];

	// Perform face culling
	if(cullingMode != None)
	{
		XMVECTOR e1 = XMVectorSubtract(vertex[2], vertex[0]);
		XMVECTOR e2 = XMVectorSubtract(vertex[2], vertex[1]);

		XMVECTOR angle = XMVector3Dot(vertex[0], XMVector3Cross(e1, e2));

		if(cullingMode == CW)
		{
			if(XMVectorGetX(angle) > 0) return 0;
		}
		else
		{
			if(XMVectorGetX(angle) < 0) return 0;
		}
	}

	for(int i = 0; i < 3; ++i)
	{
		temp[i].x = XMVectorGetX(vertex[i]);
		temp[i].y = XMVectorGetY(vertex[i]);
		temp[i].z = XMVectorGetZ(vertex[i]);
		temp[i].w = XMVectorGetW(vertex[i]);
	}

	count = 3;

	int result = 0;
	// Clip against all planes
	for(int plane = 0; plane < Planes_Count; ++plane)
	{
		count = ClipTrianglesWithPlane(temp, count, output, (ClipPlanes)plane);

		::memcpy(temp, output, sizeof(XMFLOAT4) * 64 * 3); // TODO:S_ Optimize
	}
	
	return count;
}

void TriangleClipper::ClipTriangles(XMVECTOR *vertices, int *indices, int indexCount, Culling cullingMode, std::vector<XMFLOAT4>& output)
{
	XMFLOAT4 iteration[64 * 3];
	int count = 0;
	XMVECTOR verts[3];
	for(int i = 0; i < indexCount; i += 3)
	{
		verts[0] = vertices[indices[i]];
		verts[1] = vertices[indices[i + 1]];
		verts[2] = vertices[indices[i + 2]];
		count = ClipTriangle(verts, iteration, cullingMode);
		std::copy(iteration, iteration + count, std::back_inserter(output));
	}
}