// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once

#include <DirectXMath.h>

class Camera
{
private:
	enum eDir		{ ceMove, ceStraf, ceUp };
	enum eOrient	{ cePitch , ceRoll , ceYaw };
	enum CameraType { CAM_FPS, CAM_Free };
	static const DirectX::XMFLOAT3 vDefCamOffset;

public:
	Camera();
	Camera(DirectX::XMFLOAT3 vPos, DirectX::XMFLOAT3 vCamOffset, CameraType type = CAM_FPS);
	virtual ~Camera();

	void SetOrientation(DirectX::XMFLOAT4 quatOrient);
	void SetLookAt(DirectX::XMFLOAT3 vFrom, DirectX::XMFLOAT3 vTo, DirectX::XMFLOAT3 vUp);

	void SetPos(DirectX::XMFLOAT3 vNewPos);
	void SetCameraOffset( DirectX::XMFLOAT3 vOffset);

	DirectX::XMFLOAT3 GetPos() const { return m_vectPosition; }

	void Pitch(float fAngle)	{ ApplyRotate(fAngle , cePitch); }
	void Roll(float fAngle)		{ ApplyRotate(fAngle , ceRoll);	 }
	void Yaw(float fAngle)		{ ApplyRotate(fAngle , ceYaw);	 }

	void Move(float fDistance )   { ApplyTranslation(fDistance , ceMove);  }
	void Strafe(float fDistance ) { ApplyTranslation(fDistance , ceStraf); }
	void Up(float fDistance)	  { ApplyTranslation(fDistance , ceUp);    }

	const DirectX::XMFLOAT4X4& GetViewMatrix() const;

	const DirectX::XMFLOAT3 GetAxisZ() const;
	const DirectX::XMFLOAT3 GetAxisY() const;
	const DirectX::XMFLOAT3 GetAxisX() const;
	
	const DirectX::XMFLOAT4 GetRotation() const { return m_quatOrientation; }

	static bool RotateZAxis(DirectX::XMFLOAT4* pOrientation, float Angle);
	static bool RotateYAxis(DirectX::XMFLOAT4* pOrientation, float Angle);
	static bool RotateXAxis(DirectX::XMFLOAT4* pOrientation, float Angle);
	static bool RotateAxis(DirectX::XMFLOAT4* pOrientation, DirectX::XMFLOAT3* pAxis, float Angle);
	static DirectX::XMFLOAT3* TransformVector(const DirectX::XMFLOAT4* pOrientation, DirectX::XMFLOAT3* pAxis);

	bool Slerp(DirectX::XMFLOAT4* pOrientation);

	void SetSpeed(float fSpeed) { m_fSpeed = (fSpeed < 0.0f) ? 0.0f : fSpeed; }
	void SetRPM(float fRPM) { m_fRPM = (fRPM < 0.0f) ? 0.0f : fRPM; }
	void SetSlerpSpeed(float fSlerpSpeed) {  m_fSlerpSpeed = (fSlerpSpeed >= 0.0f && fSlerpSpeed <= 1.0) ? fSlerpSpeed : m_fSlerpSpeed; }

	float GetSpeed () const		{ return m_fSpeed;		}
	float GetRPM() const		{ return m_fRPM;		}
	float GetSlerpSpeed() const { return m_fSlerpSpeed; }
	
	CameraType GetTyep() const { return m_Type; }
	void SetType(CameraType type) { m_Type = type; }

private:
	void ApplyTranslation(float fDistance, eDir ceDir);
	void ApplyRotate(float fAngle, eOrient oeOrient);
	void Init();
	void Update() const;

private:
	DirectX::XMFLOAT3 m_vectPosition;
	DirectX::XMFLOAT3 m_vectCamOffset;

	DirectX::XMFLOAT4 m_quatOrientation;
	
	mutable DirectX::XMFLOAT4X4	m_matView;

	mutable bool m_bNeedUpdated;

	float m_fSlerpSpeed;
	float m_fSpeed;
	float m_fRPM;

	CameraType m_Type;
};
