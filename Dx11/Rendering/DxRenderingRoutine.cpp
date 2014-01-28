// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#include "stdafx.h"

#include "DxRenderingRoutine.h"
#include "DxRenderer.h"

DxRenderingRoutine::DxRenderingRoutine()
	: m_Renderer(nullptr)
{}

bool DxRenderingRoutine::Initialize(Renderer* renderer)
{
	m_Renderer = boost::polymorphic_downcast<DxRenderer*>(renderer);
	
	return true;
}