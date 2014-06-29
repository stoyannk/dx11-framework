// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#include "stdafx.h"
#include "Camera.h"

using namespace DirectX;

const XMFLOAT3 Camera::vDefCamOffset(0.0f , 5.0f , -10.0f);

Camera::Camera()
{
	m_vectCamOffset = vDefCamOffset;
	m_vectPosition = XMFLOAT3( 0.0f , 0.0f , 0.0f );	// Initial position
	m_Type = CAM_FPS;

	Init();
}

Camera::Camera(XMFLOAT3 vPos, XMFLOAT3 vCamOffset, CameraType type)
{
	m_vectCamOffset = vCamOffset;
	m_vectPosition = vPos;
	m_Type = type;

	Init();
}

Camera::~Camera()
{}

void Camera::Init()
{
	m_fSlerpSpeed = 0.1f;
	m_fSpeed = 10.0f;
	m_fRPM = 60.0f; // 60 rotations per minute
	m_vectPosition.x -= m_vectCamOffset.x;
	m_vectPosition.y -= m_vectCamOffset.y;
	m_vectPosition.z -= m_vectCamOffset.z;

	XMStoreFloat4(&m_quatOrientation, XMQuaternionIdentity());
	XMStoreFloat4x4(&m_matView, XMMatrixIdentity());

	m_bNeedUpdated = true;
}

void Camera::SetPos(XMFLOAT3 vNewPos)
{
	m_vectPosition = vNewPos;
	m_bNeedUpdated = true;
}

const XMFLOAT3 Camera::GetAxisX() const
{	
	XMFLOAT3 vAxis;

	vAxis.x = m_matView._11;
	vAxis.y = m_matView._21;
	vAxis.z = m_matView._31;

	return vAxis;
}

const XMFLOAT3 Camera::GetAxisY() const
{
	XMFLOAT3 vAxis;

	vAxis.x = m_matView._12;
	vAxis.y = m_matView._22;
	vAxis.z = m_matView._32;

	return vAxis;
}

const XMFLOAT3 Camera::GetAxisZ() const
{
	XMFLOAT3 vAxis;

	vAxis.x = m_matView._13;
	vAxis.y = m_matView._23;
	vAxis.z = m_matView._33;

	return vAxis;
}

const XMFLOAT4X4& Camera::GetViewMatrix() const
{
	if (m_bNeedUpdated)
	{
		Update();
	}

	return m_matView;
}

void Camera::Update() const
{
	// 1) Build a new view matrix

	// 1.1) First calcuate Translation
	XMMATRIX matTranslation;

	matTranslation = XMMatrixTranslation(-m_vectPosition.x , 
											-m_vectPosition.y , 
											-m_vectPosition.z );

	// 1.2) Now calculate rotation, by taking the conjucate of the quaternion
	XMMATRIX matRotation;
	XMFLOAT4 rotation(-m_quatOrientation.x
					, -m_quatOrientation.y
					, -m_quatOrientation.z
					,  m_quatOrientation.w);
	matRotation = XMMatrixRotationQuaternion(XMLoadFloat4(&rotation));

	// 2) Apply rotation & translation matrix at view matrix
	XMStoreFloat4x4(&m_matView, XMMatrixMultiply(matTranslation, matRotation));

	// 3) Set flag to false, to save CPU
	m_bNeedUpdated = false;
}

void Camera::SetLookAt(XMFLOAT3 vFrom, XMFLOAT3 vTo, XMFLOAT3 vUp)
{
	XMMATRIX matTemp;

	// Setup rotation matrix
	matTemp = XMMatrixLookAtLH(XMLoadFloat3(&vFrom), XMLoadFloat3(&vTo), XMLoadFloat3(&vUp));

	m_vectPosition = vFrom;

	// Get the orientation
	XMStoreFloat4(&m_quatOrientation, XMQuaternionRotationMatrix(matTemp));

	m_bNeedUpdated = true;
}

void Camera::SetCameraOffset(XMFLOAT3 vOffset)
{
	m_vectCamOffset = vOffset;
	m_vectPosition.x -= vOffset.x;
	m_vectPosition.y -= vOffset.y;
	m_vectPosition.z -= vOffset.z;

	m_bNeedUpdated = true;
}

void Camera::SetOrientation(XMFLOAT4 quatOrient)
{
	m_quatOrientation = quatOrient;
	m_bNeedUpdated = true;
}

void Camera::ApplyTranslation(float fDistance , eDir ceDir)
{
	XMFLOAT3 vDir;

	switch (ceDir)
	{
		case ceMove:
		{
			vDir = GetAxisZ();
			break;
		}
		case ceStraf:
		{
			vDir = GetAxisX();
			break;
		}
		case ceUp:
		{
			vDir = GetAxisY();
			break;
		}
	}
	
	m_vectPosition.x += vDir.x * fDistance * m_fSpeed;
	m_vectPosition.y += vDir.y * fDistance * m_fSpeed;
	m_vectPosition.z += vDir.z * fDistance * m_fSpeed;

	m_bNeedUpdated = true;
}

void Camera::ApplyRotate(float fAngle, eOrient oeOrient)
{
	fAngle *= (m_fRPM / 60); // angle * per minute rotation

	switch( oeOrient )
	{
		case cePitch:
		{
			RotateXAxis(&m_quatOrientation , fAngle);
			break;
		}
		case ceRoll:
		{
			RotateZAxis(&m_quatOrientation , fAngle);
			break;
		}
		case ceYaw:
		{
			RotateYAxis(&m_quatOrientation , fAngle);
			break;
		}
	}

	XMStoreFloat4(&m_quatOrientation, XMQuaternionNormalize(XMLoadFloat4(&m_quatOrientation)));

	m_bNeedUpdated = true;
}

bool Camera::RotateAxis(XMFLOAT4 *pOrientation, XMFLOAT3 *pAxis, float fAngle)
{ 
	bool Success=false;

	if(pOrientation && pAxis)
	{
		XMFLOAT4 Rotation;

		XMStoreFloat4(&Rotation, XMQuaternionRotationAxis(XMLoadFloat3(TransformVector(pOrientation, pAxis)), fAngle));
				
		XMStoreFloat4(pOrientation, XMQuaternionMultiply(XMLoadFloat4(pOrientation), XMLoadFloat4(&Rotation)));
		
		Success = true;
	}

	return(Success);
}


bool Camera::RotateXAxis(XMFLOAT4 *pOrientation, float fAngle)
{ 
	bool bSuccess = false;

	if(pOrientation)
	{
		XMFLOAT4 Rotation;

		XMStoreFloat4(&Rotation, 
						XMQuaternionRotationAxis(
						XMLoadFloat3(TransformVector(pOrientation, &XMFLOAT3(1.0f, 0.0f, 0.0f))), fAngle));


		XMStoreFloat4(pOrientation, XMQuaternionMultiply(XMLoadFloat4(pOrientation), XMLoadFloat4(&Rotation)));

		bSuccess = true;
	}

	return(bSuccess);
}


bool Camera::RotateYAxis(XMFLOAT4 *pOrientation, float fAngle)
{ 
	bool bSuccess = false;

	if(pOrientation)
	{
		XMFLOAT4 Rotation;

		XMFLOAT3 rotateAround = XMFLOAT3(0.0f, 1.0f, 0.0f);
		if (m_Type != CAM_FPS)
		{
			TransformVector(pOrientation, &rotateAround);
		}

		// FPS style camera
		XMStoreFloat4(&Rotation, 
						XMQuaternionRotationAxis(
						XMLoadFloat3(&rotateAround), fAngle));


		XMStoreFloat4(pOrientation, XMQuaternionMultiply(XMLoadFloat4(pOrientation), XMLoadFloat4(&Rotation)));

		bSuccess = true;
	}

	return(bSuccess);
}


bool Camera::RotateZAxis(XMFLOAT4 *pOrientation, float fAngle)
{ 
	bool bSuccess = false;

	if(pOrientation)
	{
		XMFLOAT4 Rotation;
		
		//XMStoreFloat4(&Rotation, XMQuaternionRotationAxis(XMLoadFloat3(&XMFLOAT3(0.0f, 0.0f, 1.0f)), fAngle));
		XMStoreFloat4(&Rotation, 
						XMQuaternionRotationAxis(
						XMLoadFloat3(TransformVector(pOrientation, &XMFLOAT3(0.0f, 0.0f, 1.0f))), fAngle));

		XMStoreFloat4(pOrientation, XMQuaternionMultiply(XMLoadFloat4(pOrientation), XMLoadFloat4(&Rotation)));

		bSuccess = true;
	}

	return(bSuccess);
}

bool Camera::Slerp(XMFLOAT4 *pOrientation)
{ 
	bool bSuccess = false;

	if(pOrientation) // This is the orientation of the target
	{
		if (m_quatOrientation.x == pOrientation->x
			&& m_quatOrientation.y == pOrientation->y
			&& m_quatOrientation.z == pOrientation->z
			&& m_quatOrientation.w == pOrientation->w)
		{
			return false;
		}
		XMFLOAT4 quatInterpotedRotation;

		// Calculate SLERP 
		
		XMStoreFloat4(&quatInterpotedRotation, 
								XMQuaternionSlerp(XMLoadFloat4(&m_quatOrientation)
												, XMLoadFloat4(pOrientation), 
												m_fSlerpSpeed));

		// Apply interpolted rotation
		m_quatOrientation = quatInterpotedRotation;

		XMStoreFloat4(&m_quatOrientation, XMQuaternionNormalize(XMLoadFloat4(&m_quatOrientation)));

		bSuccess = true;
		m_bNeedUpdated = true;
	}

	return(bSuccess);
}

XMFLOAT3* Camera::TransformVector(const XMFLOAT4 *pOrientation, XMFLOAT3 *pAxis)
{
	auto vNewAxis = XMVectorSetW(XMLoadFloat3(pAxis), 1);
	auto orientation = XMLoadFloat4(pOrientation);
	vNewAxis = XMQuaternionMultiply(XMQuaternionMultiply(XMQuaternionConjugate(orientation), vNewAxis), orientation);
	
	XMStoreFloat3(pAxis, vNewAxis);

	return(pAxis);
}
