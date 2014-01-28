// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once

#include <Dx11/AppCore/Application.h>

class Renderer;

class GraphicsApplication : public Application
{
public:
	GraphicsApplication(HINSTANCE instance);

	bool Initiate(char* className, char* windowName, unsigned width, unsigned height, bool fullscreen, WNDPROC winProc, bool sRGBRT = true, int samplesCnt = 1);

	virtual void Run();

	virtual Renderer* GetRenderer() = 0;
};