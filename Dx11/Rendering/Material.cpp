// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#include "stdafx.h"

#include "Material.h"

using namespace DirectX;

#define SETFLAG(Flag, Bit) ((Flag) |= (Bit))
#define UNSETFLAG(Flag, Bit) ((Flag) &= ~(Bit))

Material::Material()
	: m_DiffuseColor(0, 0, 0)
	, m_Properties(0)
	, m_SpecularPower(0)
{}

void SetFlag(const TexturePtr& tex, MaterialProperties prop, unsigned& flags)
{
	if (tex.get())
	{
		SETFLAG(flags, prop);
	}
	else
	{
		UNSETFLAG(flags, prop);
	}
}

TexturePtr Material::GetDiffuse() const
{
	return m_Diffuse;
}

void Material::SetDiffuse(const TexturePtr& tex)
{
	m_Diffuse = tex;
	SetFlag(m_Diffuse, MP_Diffuse, m_Properties);
}

TexturePtr Material::GetNormalMap() const
{
	return m_NormalMap;
}

void Material::SetNormalMap(const TexturePtr& tex)
{
	m_NormalMap = tex;
	SetFlag(m_NormalMap, MP_Normal, m_Properties);
}

TexturePtr Material::GetAlphaMask() const
{
	return m_AlphaMask;
}

void Material::SetAlphaMask(const TexturePtr& tex)
{
	m_AlphaMask = tex;
	SetFlag(m_AlphaMask, MP_AlphaMask, m_Properties);
}

TexturePtr Material::GetSpecularMap() const
{
	return m_SpecularMap;
}

void Material::SetSpecularMap(const TexturePtr& tex)
{
	m_SpecularMap = tex;
	SetFlag(m_SpecularMap, MP_SpecularMap, m_Properties);
}

void Material::SetDiffuseColor(const DirectX::XMFLOAT3& color)
{
	m_DiffuseColor = color;
}

const XMFLOAT3& Material::GetDiffuseColor() const
{
	return m_DiffuseColor;
}

float Material::GetSpecularPower() const
{
	return m_SpecularPower;
}

void Material::SetSpecularPower(float power)
{
	m_SpecularPower = power;
	if (m_SpecularPower > 0)
	{
		SETFLAG(m_Properties, MP_SpecularPower);
	}
	else
	{
		UNSETFLAG(m_Properties, MP_SpecularPower);
	}
}

bool Material::HasProperty(MaterialProperties prop)	const
{
	return !!(m_Properties & prop);
}

