// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once

class RenderingRoutine;
class Camera;

class Renderer
{
public:
	virtual ~Renderer() {}

	virtual bool Initialize(HWND hWnd, bool fullscreen, bool sRGB = true, int samples = 1) = 0;

	virtual void SetupFrame(Camera* mainCamera, const DirectX::XMFLOAT4X4& mainProjection) = 0;

	virtual void PreRender() = 0;

	virtual void Render(float deltaTime) = 0;

	virtual void PostRender() = 0;

	virtual void AddRoutine(RenderingRoutine* routine) = 0;

	virtual void RemoveRoutine(RenderingRoutine* routine) = 0;

	virtual void ClearRoutines() = 0;

	virtual bool ReinitRoutineShading() = 0;
};