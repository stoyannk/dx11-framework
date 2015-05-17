// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#include "stdafx.h"

#ifndef MINIMAL_SIZE
#include "MeshLoader.h"
#include "Mesh.h"

#include "ObjLoader.h"
#include "RawLoader.h"

Mesh* MeshLoader::LoadMesh(DxRenderer* renderer, const std::string& filename, std::string& errors, MeshSDF* sdf)
{
	Mesh* mesh = nullptr;

	const size_t dotPos = filename.rfind('.');
	if(dotPos == filename.npos)
	{
		errors += "Unable to specify the input format for file " + filename;
		return mesh;
	}

	std::string ext(filename.begin() + dotPos, filename.end());

	boost::algorithm::to_lower(ext);
	
	if(ext == ".obj")
	{
		ObjLoader loader;

		mesh = loader.Load(renderer, filename, errors);
	}
	else if(ext == ".rmesh")
	{
		RawLoader loader;

		mesh = loader.Load(renderer, filename, errors, sdf);
	}
	else
	{
		errors += "Unsupported file format for file " + filename;
	}

	return mesh;
}

#endif
