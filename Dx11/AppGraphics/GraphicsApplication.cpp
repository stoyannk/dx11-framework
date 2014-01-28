// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#include "stdafx.h"

#include <Dx11\Rendering\Renderer.h>
#include "GraphicsApplication.h"

GraphicsApplication::GraphicsApplication(HINSTANCE instance)
	: Application(instance)
{
}

bool GraphicsApplication::Initiate(char* className, char* windowName, unsigned width, unsigned height, bool fullscreen, WNDPROC winProc, bool sRGBRT, int samplesCnt)
{
	bool createdWindow = Application::Initiate(className, windowName, width, height, fullscreen, winProc);

	if(!createdWindow)
	{
		return false;
	}

	// Initiate rendering
	if(!GetRenderer()->Initialize(GetHandle(), fullscreen, sRGBRT, samplesCnt))
	{
		SLOG(Sev_Error, Fac_Rendering, "Unable to initialize rendering");
		return false;
	}

	return true;
}

void GraphicsApplication::Run()
{
	Application::Run();
}
