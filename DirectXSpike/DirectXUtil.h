#pragma once

void dxPowerUp( LPDIRECT3DDEVICE9 device );
void dxClear(int count,CONST D3DRECT* pRects, DWORD flags, D3DCOLOR color, float z, DWORD stencil);
void dxStencilFunc( D3DCMPFUNC func, DWORD ref, DWORD mask );
void dxStencilOp( D3DSTENCILOP stenPass, D3DSTENCILOP stenFail, D3DSTENCILOP zFail );
void dxCullMode( D3DCULL cullMode );
void dxFillMode( D3DFILLMODE fillMode );
void dxEnableStencilTest( bool enable=true );
void dxEnableZTest( bool enable=true, bool useW=false ); 
void dxEnableZWrite( bool enable=true );
void dxEnableDithering( bool enable=true );
void dxEnableAlphaTest( bool enable=true );
void dxEnableAlphaBlend( bool enable=true );
void dxEnableLineAA( bool enable=true );
void dxEnableMultisample( bool enable=true );