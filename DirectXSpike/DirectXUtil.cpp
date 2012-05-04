/********************************************************************
	created:	2012/04/16
	filename: 	DirectXUtil.cpp
	author:		Matthew Alford
	
	purpose:	
*********************************************************************/
#include "stdafx.h"
#include "directxutil.h"

static LPDIRECT3DDEVICE9 s_device;

#define __d	s_device

void dxPowerUp( LPDIRECT3DDEVICE9 device )
{
	HR(__d = device);
}

void dxClear(int count,CONST D3DRECT* pRects, DWORD flags, D3DCOLOR color, float z, DWORD stencil)
{
	HR(__d->Clear(count, pRects, flags, color, z, stencil));
}

void dxStencilFunc( D3DCMPFUNC func, DWORD ref, DWORD mask )
{
	HR(__d->SetRenderState(D3DRS_STENCILFUNC, func));
	HR(__d->SetRenderState(D3DRS_STENCILREF, ref));
	HR(__d->SetRenderState(D3DRS_STENCILMASK, mask));
}

void dxStencilOp( D3DSTENCILOP stenPass, D3DSTENCILOP stenFail, D3DSTENCILOP zFail )
{
	HR(__d->SetRenderState(D3DRS_STENCILZFAIL, zFail ));
	HR(__d->SetRenderState(D3DRS_STENCILFAIL, stenFail ));
	HR(__d->SetRenderState(D3DRS_STENCILFAIL, stenPass ));
}

void dxEnableDithering( bool enable/*=true*/ )
{
	HR(__d->SetRenderState(D3DRS_DITHERENABLE, enable));
}

void dxEnableAlphaTest( bool enable/*=true*/ )
{
	HR(__d->SetRenderState(D3DRS_ALPHATESTENABLE, enable));
}

void dxEnableAlphaBlend( bool enable/*=true*/ )
{
	HR(__d->SetRenderState(D3DRS_ALPHABLENDENABLE, enable));
}

void dxCullMode( D3DCULL cullMode )
{
	HR(__d->SetRenderState(D3DRS_CULLMODE, cullMode));
}

void dxEnableZTest( bool enable/*=true*/, bool useW/*=false*/ )
{
	if( enable )
	{
		HR(__d->SetRenderState(D3DRS_ZENABLE, useW ? D3DZB_USEW : D3DZB_TRUE));
	}
	else
	{
		HR(__d->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE));
	}
}

void dxEnableZWrite( bool enable/*=true*/ )
{
	HR(__d->SetRenderState(D3DRS_ZWRITEENABLE, enable));
}

void dxEnableStencilTest( bool enable/*=true*/ )
{
	HR(__d->SetRenderState(D3DRS_STENCILENABLE, TRUE));
}

void dxFillMode( D3DFILLMODE fillMode )
{
	HR(__d->SetRenderState(D3DRS_FILLMODE, fillMode));
}

void dxEnableLineAA( bool enable/*=true*/ )
{
	HR(__d->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, enable));
}

void dxEnableMultisample( bool enable/*=true*/ )
{
	HR(__d->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, enable));
}

void dxSetStreamIndexedSource(unsigned int num, IDirect3DIndexBuffer9* ind, IDirect3DVertexBuffer9* pStreamData, unsigned int offsetInBytes, unsigned int stride)
{
	HR(__d->SetStreamSource(num, pStreamData, offsetInBytes, stride));
	HR(__d->SetIndices(ind));
}

void dxFVF(DWORD format)
{
	HR(__d->SetFVF(format));
}

void dxDrawIndexedPrimitive(D3DPRIMITIVETYPE type, int baseVertexIndex, unsigned int minVertexIndex, unsigned int numVertices, unsigned int startIndex, unsigned int primCount)
{
	HR(__d->DrawIndexedPrimitive(type, baseVertexIndex, minVertexIndex, numVertices, startIndex, primCount));
}

void dxSetSamplerState(DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD value)
{
	HR(__d->SetSamplerState(sampler, type, value));
}

void dxCreateVertexBuffer( unsigned int len, DWORD usage, DWORD fvf, D3DPOOL pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle )
{
	HR(__d->CreateVertexBuffer(len, usage, fvf, pool, ppVertexBuffer, pSharedHandle));
}

void dxCreateIndexBuffer(unsigned len, DWORD usage, D3DFORMAT format, D3DPOOL pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle)
{
	HR(__d->CreateIndexBuffer(len, usage, format, pool, ppIndexBuffer, pSharedHandle));
}

void dxCreateSphere(float radius, unsigned int slices, unsigned int stacks, LPD3DXMESH* ppMesh, LPD3DXBUFFER* ppAdjacency)
{
	HR(D3DXCreateSphere(__d, radius, slices, stacks, ppMesh, ppAdjacency));
}

void dxCloneMeshFVF(LPD3DXMESH mesh, DWORD options, DWORD fvf, LPD3DXMESH* ppCloneMesh)
{
	HR(mesh->CloneMeshFVF(options, fvf, __d, ppCloneMesh));
}


void dxPresent(const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion)
{
	HR(__d->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion));
}

void dxCreateTeapot(LPD3DXMESH* ppMesh, LPD3DXBUFFER* ppAdjacency)
{
	HR(D3DXCreateTeapot(__d, ppMesh, ppAdjacency));
}

void dxCreateBox(float width, float height, float depth, LPD3DXMESH* ppMesh, LPD3DXBUFFER* ppAdjacency)
{
	HR(D3DXCreateBox( __d, width, height, depth, ppMesh, ppAdjacency ));
}

void dxCreateCylinder( float radius1, float radius2, float len, unsigned int slices, unsigned int stacks, LPD3DXMESH* ppMesh, LPD3DXBUFFER* ppAdjacency)
{
	HR(D3DXCreateCylinder( __d, radius1, radius2, len, slices, stacks, ppMesh, ppAdjacency ));
}

void dxCreateMeshFVF(DWORD numFaces, DWORD numVertices, DWORD options, DWORD fvf, LPD3DXMESH* ppMesh)
{
	HR(D3DXCreateMeshFVF( numFaces, numVertices, options, fvf, __d, ppMesh ));
}

void dxCreateTextureFromFile(const char* name, LPDIRECT3DTEXTURE9* ppTexture)
{
	HR(D3DXCreateTextureFromFile(__d, name, ppTexture));
}