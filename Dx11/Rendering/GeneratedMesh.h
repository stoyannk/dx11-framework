// Copyright (c) 2011-2014, Stoyan Nikolov
// All rights reserved.
// This software is governed by a permissive BSD-style license. See LICENSE.
#pragma once

#include "Material.h"

class GeneratedMesh;
typedef std::shared_ptr<GeneratedMesh> GeneratedMeshPtr;

class GeneratedMesh
{
public:
	static GeneratedMeshPtr Create(
		ID3D11Device* device,
		unsigned bufferSize,
		const std::string& generatingFunction,
		const DirectX::XMINT3& dispatch);
	virtual ~GeneratedMesh();

	void SetDynamic(bool d)
	{
		m_Dynamic = d;
	}

	bool IsDynamic() const
	{
		return m_Dynamic;
	}

	const Material& GetMaterial() const
	{
		return m_Material;
	}

	void SetMaterial(const Material& m)
	{
		m_Material = m;
	}

	void SetGenerator(const std::string& generatingFunction,
		const DirectX::XMINT3& dispatch)
	{
		m_GeneratingFunction = generatingFunction;
		m_Dispatch = dispatch;
	}

	const std::string& GetGeneratingFunction() const
	{
		return m_GeneratingFunction;
	}

	const DirectX::XMINT3& GetDispatch() const
	{
		return m_Dispatch;
	}

	unsigned GetBufferSize() const
	{
		return m_BufferSize;
	}

	ID3D11Buffer* GetVertexBuffer() const
	{
		return m_GeneratedBuffer.Get();
	}
	ID3D11Buffer* GetIndexBuffer() const
	{
		return m_GeneratedIB.Get();
	}
	ID3D11Buffer* GetCountersBuffer() const
	{
		return m_CountersBuffer.Get();
	}
	ID3D11Buffer* GetIndirectBuffer() const
	{
		return m_IndirectBuffer.Get();
	}
	
	ID3D11UnorderedAccessView* GetVertexBufferUAV() const
	{
		return m_GeneratedUAV.Get();
	}
	ID3D11UnorderedAccessView* GetIndexBufferUAV() const
	{
		return m_GeneratedIBUAV.Get();
	}
	ID3D11UnorderedAccessView* GetCountersBufferUAV() const
	{
		return m_CountersUAV.Get();
	}
	ID3D11UnorderedAccessView* GetIndirectBufferUAV() const
	{
		return m_IndirectUAV.Get();
	}

	ID3D11ShaderResourceView* GetCountersSRV() const
	{
		return m_CountersSRV.Get();
	}
private:
	GeneratedMesh(unsigned buffSize,
		const std::string& generatingFunction,
		const DirectX::XMINT3& dispatch);

	unsigned m_BufferSize;
	std::string m_GeneratingFunction;
	DirectX::XMINT3 m_Dispatch;

	ReleaseGuard<ID3D11Buffer> m_GeneratedBuffer;
	ReleaseGuard<ID3D11UnorderedAccessView> m_GeneratedUAV;

	ReleaseGuard<ID3D11Buffer> m_GeneratedIB;
	ReleaseGuard<ID3D11UnorderedAccessView> m_GeneratedIBUAV;

	ReleaseGuard<ID3D11Buffer> m_CountersBuffer;
	ReleaseGuard<ID3D11UnorderedAccessView> m_CountersUAV;
	ReleaseGuard<ID3D11ShaderResourceView> m_CountersSRV;

	ReleaseGuard<ID3D11Buffer> m_IndirectBuffer;
	ReleaseGuard<ID3D11UnorderedAccessView> m_IndirectUAV;

	Material m_Material;
	bool m_Dynamic;
};