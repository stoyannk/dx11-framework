// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once

#include "DxRenderer.h"
#include "RenderingRoutine.h"

class DxRenderingRoutine : public RenderingRoutine
{
public:
	DxRenderingRoutine();
	virtual bool Initialize(Renderer* renderer);

protected:
	DxRenderer* m_Renderer;
};