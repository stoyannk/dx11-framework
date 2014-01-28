// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#include "stdafx.h"

#include "MeshSaver.h"
#include "Mesh.h"

#include "RawSaver.h"

bool MeshSaver::SaveMesh(DxRenderer* renderer, Mesh* mesh, const std::string& filename, std::string& errors)
{
	const size_t dotPos = filename.rfind('.');
	if(dotPos == filename.npos)
	{
		errors += "Unable to specify the output format for file " + filename;
		return false;
	}

	std::string ext(filename.begin() + dotPos, filename.end());

	boost::algorithm::to_lower(ext);
	if(ext != ".rmesh")
	{
		errors += "Unsupported file format for file " + filename;
		return false;
	}

	RawSaver saver;

	return saver.Save(renderer, mesh, filename, errors);
}
