// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#include "stdafx.h"

#include "DxGraphicsApplication.h"

using namespace DirectX;

DxGraphicsApplication::DxGraphicsApplication(HINSTANCE instance)
	: GraphicsApplication(instance)
	, m_Renderer(new DxRenderer)
{
	XMStoreFloat4x4(&m_Projection, XMMatrixIdentity());
}
	
DxGraphicsApplication::~DxGraphicsApplication()
{
	SafeDelete(m_Renderer);
}

Renderer* DxGraphicsApplication::GetRenderer()
{
	return m_Renderer;
}

Camera* DxGraphicsApplication::GetMainCamera()
{
	return &m_MainCamera;
}

const XMFLOAT4X4& DxGraphicsApplication::GetProjection() const
{
	return m_Projection;
}

void DxGraphicsApplication::SetProjection(float fov, float aspect, float nearZ, float farZ)
{
	XMStoreFloat4x4(&m_Projection, XMMatrixPerspectiveFovLH(fov, aspect, nearZ, farZ));
}

void DxGraphicsApplication::Run()
{
	GraphicsApplication::Run();
	
	GetRenderer()->SetupFrame(GetMainCamera(), GetProjection());

	PreRender();
	GetRenderer()->PreRender();
	GetRenderer()->Render(GetLastFrameTime());
	GetRenderer()->PostRender();
	PostRender();
}
