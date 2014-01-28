// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once

class Mesh;
class DxRenderer;

class MeshSaver
{
public:
	static bool SaveMesh(DxRenderer* renderer, Mesh* mesh, const std::string& filename, std::string& errors);
};