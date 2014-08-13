// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#include "stdafx.h"
#include "Entity.h"
#include <Utilities/Aligned.h>

template<typename MeshT>
Transformed<MeshT>::Transformed()
: Rotation(DirectX::XMVectorZero())
, Position(DirectX::XMFLOAT3A(0, 0, 0))
, Scale(1.f)
{}

template<typename MeshT>
Transformed<MeshT>::~Transformed()
{}

template struct Transformed<MeshPtr>;
template struct Transformed<GeneratedMeshPtr>;
