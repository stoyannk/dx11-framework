// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once

class Mesh;
class DxRenderer;
class MeshSDF;

class RawLoader
{
public:
	Mesh* Load(DxRenderer* renderer, const std::string& filename, std::string& errors, MeshSDF* sdf = nullptr);
};