// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#include "stdafx.h"
#include "SoftwareRasterizer.h"

using namespace DirectX;

SoftwareRasterizer::SoftwareRasterizer()
	: m_CullingMode(TriangleClipper::CW)
{}

void SoftwareRasterizer::SetRenderTarget(void* bytes, int pitch, int cols, int rows, PixelFormat format)
{
	assert(pitch == cols*4); // TODO:S_ support variable pitch
	
	if(m_RenderTarget.Rows != rows)
	{
		m_ActivationTable.reset(new std::vector<SoftwareRasterizer::Edge*>[rows]);
	}
		
	m_RenderTarget.Bytes = bytes;
	m_RenderTarget.Pitch = pitch;
	m_RenderTarget.Cols = cols;
	m_RenderTarget.Rows = rows;
	m_RenderTarget.Pf = format;
}

void SoftwareRasterizer::SetCullingMode(TriangleClipper::Culling culling)
{
	m_CullingMode = culling;
}

TriangleClipper::Culling SoftwareRasterizer::GetCullingMode() const
{
	return m_CullingMode;
}

void SoftwareRasterizer::SetGlobalMatrices(const XMFLOAT4X4& view, const XMFLOAT4X4& projection)
{
	XMMATRIX viewM = XMLoadFloat4x4(&view);
	XMMATRIX projM = XMLoadFloat4x4(&projection);

	XMMATRIX viewProj = XMMatrixMultiply(viewM, projM);
	XMStoreFloat4x4(&m_ViewProj, viewProj);
}

void SoftwareRasterizer::Triangle::CalcProperties()
{
	CurrentZ = 0;

	// TODO: This is obviuously very bad
	XMVECTOR plane = XMPlaneFromPoints(XMLoadFloat3(&Vertices[0])
									 , XMLoadFloat3(&Vertices[1])
									 , XMLoadFloat3(&Vertices[2]));

	A = XMVectorGetX(plane);
	B = XMVectorGetY(plane);
	C = XMVectorGetZ(plane);
}

void SoftwareRasterizer::Triangle::InterpolateZ(Edge* e, int y)
{
	const int yterm = int(e->Vertex1.y - e->Vertex2.y);
	if(yterm != 0)
	{
		CurrentZ = e->Vertex1.z - (e->Vertex1.z - e->Vertex2.z) * ((e->Vertex1.y - y) / (yterm));
	}
	else
	{
		CurrentZ = e->Vertex1.z;
	}
}

void SoftwareRasterizer::Triangle::IncreaseZ()
{
	// assumes dx = 1
	CurrentZ += -A/C; // TODO:S_ cache
}
  
void SoftwareRasterizer::Edge::CalcProperties()
{
	XMFLOAT3* minV = nullptr;
	minV = Vertex1.y < Vertex2.y ? &Vertex1 : &Vertex2; 
	ymin = int(minV->y);
	assert(ymin >= 0);
	ymax = int(std::max(Vertex1.y, Vertex2.y));

	x = minV->x;

	XMFLOAT3* maxV = (minV == &Vertex1 ? &Vertex2 : &Vertex1);

	int dx = int(maxV->x - minV->x);
	if(dx)
	{
		oneoverm = 1.f / (float(maxV->y - minV->y) / dx);
	}
	else
	{
		oneoverm = 0;
	}
}

template <typename T>
bool is_near(T&& a, T&& b, T epsilon = std::numeric_limits<T>::epsilon())
{
	return std::abs(a - b) <= epsilon;
}

template <typename T>
T clamp(T value, T min, T max)
{
	return std::min<T>(max, std::max<T>(min, value));
}

#define TO_ABGR(r, g, b, a) (((unsigned char)a << 24) | ((unsigned char)b << 16) | ((unsigned char)g << 8) | ((unsigned char)r))
#define FLOAT4_TO_COLOR(COLOR) ((SoftwareRasterizer::Color)(TO_ABGR((COLOR[0] * 255), (COLOR[1] * 255), (COLOR[2] * 255), (COLOR[3] * 255))))

#define ERROR_TOLERANCE 0.00001f

bool SoftwareRasterizer::DrawIndexedTriangles(const XMFLOAT4X4& world, PositionColorVertex* vertices, int* indices, int indicesCount)
{
	// Build WVP matrix
	XMMATRIX WVP = XMLoadFloat4x4(&m_ViewProj);
	XMMATRIX World = XMLoadFloat4x4(&world);

	WVP = XMMatrixMultiply(World, WVP);

	// TODO:S_ pools?
	boost::scoped_array<XMVECTOR> transformedVertices(new XMVECTOR[indicesCount]);
	boost::scoped_array<bool> isTransformed(new bool[indicesCount]());
	
	unsigned long* surface = (unsigned long*)m_RenderTarget.Bytes;

	PositionColorVertex* currentVertex = nullptr;
	XMVECTOR vertex[3];
	XMFLOAT4 clippedVertices[64 * 3];
	float x,y,z,w;
	std::vector<Triangle> triangles;
	std::vector<Edge> edges;
	edges.reserve(indicesCount * 2); // Heuristic for horizontal edges
	triangles.reserve(indicesCount / 3 * 2);
	for(int i = 0; i < indicesCount; i += 3)
	{
		// Transform all vertices (or find them in cache)
		for(int vi = 0; vi < 3; ++vi)
		{
			currentVertex = vertices + indices[i + vi];

			if(!isTransformed[i + vi])
			{
				vertex[vi] = XMLoadFloat4(&XMFLOAT4(currentVertex->x, currentVertex->y, currentVertex->z, 1.f));
				vertex[vi] = XMVector4Transform(vertex[vi], WVP);
			}
			else
			{
				vertex[vi] = transformedVertices[i + vi];
			}
		}

		int clippedVertsCount = TriangleClipper::ClipTriangle(vertex, clippedVertices, m_CullingMode);

		for(int clippedVert = 0; clippedVert < clippedVertsCount; clippedVert += 3)
		{
			XMFLOAT3 points[3];
			triangles.push_back(Triangle());
			for(int vi = 0; vi < 3; ++vi)
			{
				x =	clippedVertices[clippedVert + vi].x;
				y =	clippedVertices[clippedVert + vi].y;
				z = clippedVertices[clippedVert + vi].z;
				w = clippedVertices[clippedVert + vi].w;
				x /= w; y /= w; z /= w;

				assert(x >= -1 - ERROR_TOLERANCE
					&& x <= 1 + ERROR_TOLERANCE 
					&& y >= -1 - ERROR_TOLERANCE
					&& y <= 1 + ERROR_TOLERANCE
					&& z >= -1 - ERROR_TOLERANCE
					&& z <= 1 + ERROR_TOLERANCE);

				x = clamp(x, -1.f, 1.f);
				y = clamp(y, -1.f, 1.f);
				z = clamp(z, -1.f, 1.f);

				// Draw on rt
				int xviewport = int(((x + 1.f) / 2.f) * (m_RenderTarget.Cols - 1));
				int yviewport = int(((1.f - y) / 2.f) * (m_RenderTarget.Rows - 1));
				points[vi].x = float(xviewport); points[vi].y = float(yviewport);	points[vi].z = z;

				//surface[yviewport * m_RenderTarget.Cols + xviewport] = 0xFF0000FF; // DEBUG

				triangles.back().Vertices[vi] = XMFLOAT3(float(xviewport), float(yviewport), z);
			}
			triangles.back().Color = FLOAT4_TO_COLOR(currentVertex->color);
			triangles.back().CalcProperties();

			// Fill
			XMFLOAT2 Vertex;
			{
				edges.push_back(Edge());
				edges.back().Vertex1 = points[0];
				edges.back().Vertex2 = points[1];
				edges.back().CalcProperties();
				edges.back().triagId = triangles.size() - 1;

				edges.push_back(Edge());
				edges.back().Vertex1 = points[0];
				edges.back().Vertex2 = points[2];
				edges.back().CalcProperties();			
				edges.back().triagId = triangles.size() - 1;

				edges.push_back(Edge());
				edges.back().Vertex1 = points[1];
				edges.back().Vertex2 = points[2];
				edges.back().CalcProperties();			
				edges.back().triagId = triangles.size() - 1;
			}
		}
		++currentVertex;
	}

	// Duplicate all horizontal edges
	std::vector<Edge> duplicates;
	for(std::vector<Edge>::iterator it = edges.begin(); it != edges.end(); ++it)
	{
		if(it->ymin == it->ymax)
		{
			++(it->ymax);
			it->oneoverm = 0;
			Edge e(*it);
			e.x = std::max(e.Vertex1.x, e.Vertex2.x);
			duplicates.push_back(e);
		}
	}
	// merge edges
	edges.insert(edges.end(), duplicates.begin(), duplicates.end()); //TODO: OPTIMIZE

	if(edges.size())
	{
		ScanlineFill(&edges[0], edges.size(), &triangles[0]);
	}
	return true;
}

void SoftwareRasterizer::ScanlineFill(Edge* __restrict edges, int edgesCount, Triangle* __restrict triangles)
{
	unsigned long* __restrict surface = (unsigned long* __restrict)m_RenderTarget.Bytes;

	typedef std::vector<Edge*> AELType;
	static AELType AEL;
	static TrianglesSet AT;

	auto OnNewEdge = []	(TrianglesSet& active, Triangle* triags, Edge* e, int line) -> bool
	{
		Triangle* __restrict triangle = &triags[e->triagId];
		SoftwareRasterizer::TrianglesSet::const_iterator it = std::find(active.begin(), active.end(), triangle);
		if(it == active.end())
		{
			active.push_back(triangle);
			triangle->InterpolateZ(e, line);
			return true;
		}
		else
		{
			active.erase(it);
			return false;
		}
	};

	struct AEListRemovePredicate
	{
		bool operator()(Edge* e) const
		{
			return Line == e->ymax && e->ymax != e->ymin;
		}

		int Line;
	};
	struct AEListSorter
	{
		bool operator()(Edge* e1, Edge* e2)
		{
			return e1->x < e2->x;
		}
	};

	// Fill activation table
	for(int edge = 0; edge < edgesCount; ++edge)
	{
		m_ActivationTable[edges[edge].ymin].push_back(&edges[edge]);
	}

	// Scan
	int startx = 0;
	int endx = 0;
	bool hasAddedTriag = false;
	AEListRemovePredicate removePredicate;
	AEListSorter sorter;
	for(int line = 0; line < m_RenderTarget.Rows; ++line)
	{
		if(!m_ActivationTable[line].empty())
		{
			AEL.insert(AEL.end(), m_ActivationTable[line].begin(), m_ActivationTable[line].end());
			m_ActivationTable[line].clear();
		}
	
		removePredicate.Line = line;
		AEL.erase(std::remove_if(AEL.begin(), AEL.end(), removePredicate), AEL.end());
	
		std::sort(AEL.begin(), AEL.end(), sorter);
	
		for(AELType::iterator edge = AEL.begin(); edge != AEL.end(); )
		{
			AELType::iterator e1 = edge++;
		
			OnNewEdge(AT, triangles, (*e1), line);
		
			if(AT.empty())
			{
				(*e1)->x += (*e1)->oneoverm;
				continue;
			}
			
			AELType::iterator e2 = edge;
			
			// Draw pairs
			startx = (int)(*e1)->x;
			endx = (int)(*e2)->x;
		
			assert(startx <= endx);
		
			FillInterval(line, startx, endx, AT);
		
			(*e1)->x += (*e1)->oneoverm;
		}
	}

	AEL.clear();
	AT.clear();
}

void SoftwareRasterizer::FillInterval(int line, int startx, int endx, TrianglesSet& AT)
{
	unsigned long* __restrict surface = (unsigned long* __restrict)m_RenderTarget.Bytes;

	for(int i = startx; i < endx; ++i)
	{
		TrianglesSet::iterator minit = AT.begin();
		// Get the color with the minimal Z
		TrianglesSet::iterator it = minit;
		++it;
		for(;it != AT.end(); ++it) //TODO:S_ Optimize
		{
			if((*minit)->CurrentZ > (*it)->CurrentZ)
			{
				minit = it;
			}
		}

		surface[line * m_RenderTarget.Cols + i] = (*minit)->Color;

		// Inc Z for all triangles
		std::for_each(AT.begin(), AT.end(), std::mem_fun(&Triangle::IncreaseZ));
	}
}
