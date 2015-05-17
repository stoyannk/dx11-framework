// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#include "stdafx.h"
#include "MeshSDF.h"
#include "MathConv.h"
#include <numeric>
#include <utility>

#include <ThirdParty/glm/glm/gtc/matrix_transform.hpp>

using namespace DirectX;

MeshSDF::MeshSDF(const glm::u32vec3& dims)
	: m_Dims(dims)
	, m_SDF(new char[dims.x * dims.y * dims.z])
{}

MeshSDF::MeshSDF(MeshSDF&& other)
	: m_Dims(other.m_Dims)
{
	m_SDF.swap(other.m_SDF);
}

MeshSDF& MeshSDF::operator=(MeshSDF&& other)
{
	std::swap(m_SDF, other.m_SDF);
	std::swap(m_Dims, other.m_Dims);
	return *this;
}

void BruteForceSDFGen(
	const StandardVertex* vertices,
	const unsigned** indices,
	const unsigned* indexSizes,
	unsigned subsetCount,
	char* outSDF,
	const glm::u32vec3& dims)
{
	const auto totalTriags = std::accumulate(indexSizes, indexSizes + subsetCount, 0u) / 3;

	std::unique_ptr<glm::mat4[]> matrices 
		= std::make_unique<glm::mat4[]>(totalTriags);

	// Transform all triangles
	unsigned triagIndex = 0;
	for (auto subset = 0u; subset < subsetCount; ++subset)
	for (auto i = 0u; i < indexSizes[subset]; i += 3, ++triagIndex)
	{
		const auto i0 = indices[subset][i];
		const auto i1 = indices[subset][i + 1];
		const auto i2 = indices[subset][i + 2];

		const auto v0v1 = glm::vec3(toVec3(vertices[i1].Position)) - glm::vec3(toVec3(vertices[i0].Position));
		const auto v1v2 = glm::vec3(toVec3(vertices[i2].Position)) - glm::vec3(toVec3(vertices[i1].Position));

		const auto normal = glm::normalize(glm::cross(v0v1, v1v2));

		const glm::mat4 t1 = glm::translate(glm::mat4(1.0f), toVec3(vertices[i0].Position));
		
		const auto cosa = -normal.x;
		const auto sina = sin(acos(-normal.x));

		const glm::mat4 t2 = glm::mat4(
			cosa, -normal.y * sina, -normal.z*sina, 0,
			normal.y*sina, cosa + pow(normal.z, 2)*(1 - cosa), -normal.z*normal.y*(1-cosa), 0,
			normal.z*sina, -normal.y*normal.z*(1 - cosa), cosa + pow(normal.y, 2)*(1-cosa), 0,
			0, 0, 0, 1
		);

		matrices[triagIndex] = t2*t1;
	}

	// Check distances w/ matrices
	for (auto z = 0u; z < dims.z; ++z)
	for (auto y = 0u; y < dims.y; ++y)
	for (auto x = 0u; x < dims.x; ++x)
	{

	}
}

void MeshSDF::Populate(const StandardVertex* vertices,
	const unsigned** indices,
	const unsigned* indexSizes,
	unsigned subsetCount,
	const glm::vec3& aabbMin,
	const glm::vec3& aabbMax)
{
	BruteForceSDFGen(vertices,
		indices,
		indexSizes,
		subsetCount,
		m_SDF.get(),
		m_Dims);
}
