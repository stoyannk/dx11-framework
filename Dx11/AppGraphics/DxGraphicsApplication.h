// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once

#include "GraphicsApplication.h"
#include <Dx11/Rendering/DxRenderer.h>
#include <Dx11/Rendering/Camera.h>

class DxGraphicsApplication : public GraphicsApplication
{
public:
	DxGraphicsApplication(HINSTANCE instance);
	virtual ~DxGraphicsApplication();

	virtual Renderer* GetRenderer() override;

	virtual Camera* GetMainCamera();

	const DirectX::XMFLOAT4X4& GetProjection() const;

	virtual void Run();

	void SetProjection(float fov, float aspect, float nearZ, float farZ);

protected:
	DxRenderer* m_Renderer;
	
	Camera m_MainCamera;

	DirectX::XMFLOAT4X4 m_Projection;
};