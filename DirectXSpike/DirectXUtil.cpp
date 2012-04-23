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
	__d = device;
}

void dxClear(int count,CONST D3DRECT* pRects, DWORD flags, D3DCOLOR color, float z, DWORD stencil)
{
	__d->Clear(count, pRects, flags, color, z, stencil);
}

void dxStencilFunc( D3DCMPFUNC func, DWORD ref, DWORD mask )
{
	__d->SetRenderState(D3DRS_STENCILFUNC, func);
	__d->SetRenderState(D3DRS_STENCILREF, ref);
	__d->SetRenderState(D3DRS_STENCILMASK, mask);
}

void dxStencilOp( D3DSTENCILOP stenPass, D3DSTENCILOP stenFail, D3DSTENCILOP zFail )
{
	__d->SetRenderState(D3DRS_STENCILZFAIL, zFail );
	__d->SetRenderState(D3DRS_STENCILFAIL, stenFail );
	__d->SetRenderState(D3DRS_STENCILFAIL, stenPass );
}

void dxEnableDithering( bool enable/*=true*/ )
{
	__d->SetRenderState(D3DRS_DITHERENABLE, enable);
}

void dxEnableAlphaTest( bool enable/*=true*/ )
{
	__d->SetRenderState(D3DRS_ALPHATESTENABLE, enable);
}

void dxEnableAlphaBlend( bool enable/*=true*/ )
{
	__d->SetRenderState(D3DRS_ALPHABLENDENABLE, enable);
}

void dxCullMode( D3DCULL cullMode )
{
	__d->SetRenderState(D3DRS_CULLMODE, cullMode);
}

void dxEnableZTest( bool enable/*=true*/, bool useW/*=false*/ )
{
	if( enable )
		__d->SetRenderState(D3DRS_ZENABLE, useW ? D3DZB_USEW : D3DZB_TRUE);
	else
		__d->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
}

void dxEnableZWrite( bool enable/*=true*/ )
{
	__d->SetRenderState(D3DRS_ZWRITEENABLE, enable);
}

void dxEnableStencilTest( bool enable/*=true*/ )
{
	__d->SetRenderState(D3DRS_STENCILENABLE, TRUE);
}

void dxFillMode( D3DFILLMODE fillMode )
{
	__d->SetRenderState(D3DRS_FILLMODE, fillMode);
}

void dxEnableLineAA( bool enable/*=true*/ )
{
	__d->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, enable);
}

void dxEnableMultisample( bool enable/*=true*/ )
{
	__d->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, enable);
}