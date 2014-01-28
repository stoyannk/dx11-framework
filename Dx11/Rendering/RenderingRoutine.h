// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once

class Renderer;

class RenderingRoutine
{
public:
	virtual ~RenderingRoutine() {}

	virtual bool Initialize(Renderer* renderer) = 0;

	virtual bool PreRender() {return true;}

	virtual bool Render(float deltaTime) {return true;};

	virtual bool PostRender() {return true;}

	virtual bool ReinitShading() {return true;}
};