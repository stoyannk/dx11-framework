// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once

#include <ThirdParty/glm/glm/glm.hpp>
#include "VertexTypes.h"

class MeshSDF
{
public:
	MeshSDF(const glm::u32vec3& dims);
	MeshSDF(MeshSDF&& other);
	MeshSDF& operator=(MeshSDF&& other);

	char* Data()
	{
		return m_SDF.get();
	}

	void Populate(const StandardVertex* vertices,
		const unsigned** indices,
		const unsigned* indexSizes,
		unsigned subsetCount,
		const glm::vec3& aabbMin,
		const glm::vec3& aabbMax);

private:
	MeshSDF(const MeshSDF&) = delete;
	MeshSDF& operator=(const MeshSDF&) = delete;

	std::unique_ptr<char []> m_SDF;
	glm::u32vec3 m_Dims;
};

typedef std::vector<MeshSDF> MeshSDFVec;
