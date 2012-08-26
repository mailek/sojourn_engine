#pragma once
/********************************************************************
	created:	2012/05/22
	filename: 	DirectXUtil.h
	author:		Matthew Alford
	
	purpose:	DirectX Device Wrapper and API
*********************************************************************/

#include <d3d9.h>
#include <d3dx9.h>

//////////////////////////////////////
// Type Definitions
//////////////////////////////////////

#define Color_4 D3DXCOLOR


//////////////////////////////////////
// Function Declarations
//////////////////////////////////////

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
void dxSetStreamIndexedSource(unsigned int num, IDirect3DIndexBuffer9* ind, IDirect3DVertexBuffer9* pStreamData, unsigned int offsetInBytes, unsigned int stride);
void dxFVF(DWORD format);
void dxDrawIndexedPrimitive(D3DPRIMITIVETYPE type, int baseVertexIndex, unsigned int minVertexIndex, unsigned int numVertices, unsigned int startIndex, unsigned int primCount);
void dxSetSamplerState(DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD value);
void dxCreateVertexBuffer(unsigned int len, DWORD usage, DWORD fvf, D3DPOOL pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle);
void dxCreateIndexBuffer(unsigned len, DWORD usage, D3DFORMAT format, D3DPOOL pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle);
void dxCreateSphere(float radius, unsigned int slices, unsigned int stacks, LPD3DXMESH* ppMesh, LPD3DXBUFFER* ppAdjacency);
void dxCloneMeshFVF(LPD3DXMESH mesh, DWORD options, DWORD fvf, LPD3DXMESH* ppCloneMesh);
void dxPresent(const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion);
void dxCreateTeapot(LPD3DXMESH* ppMesh, LPD3DXBUFFER* ppAdjacency);
void dxCreateSphere(float radius, unsigned int slices, unsigned stacks, LPD3DXMESH* ppMesh, LPD3DXBUFFER* ppAdjacency);
void dxCreateBox(float width, float height, float depth, LPD3DXMESH* ppMesh, LPD3DXBUFFER* ppAdjacency);
void dxCreateCylinder(float radius1, float radius2, float len, unsigned int slices, unsigned int stacks, LPD3DXMESH* ppMesh, LPD3DXBUFFER* ppAdjacency);
void dxCreateMeshFVF(DWORD numFaces, DWORD numVertices, DWORD options, DWORD fvf, LPD3DXMESH* ppMesh);
void dxCreateTextureFromFile(const char* name, LPDIRECT3DTEXTURE9* ppTexture);