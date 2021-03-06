#pragma once
/********************************************************************
	created:	2012/04/16
	filename: 	VertexTypes.h
	author:		Matthew Alford
	
	purpose:	Defines the various vertex definitions used throughout
				the render engine.
*********************************************************************/

//////////////////////////////////////
// Terrain.cpp
//////////////////////////////////////
#define TEX_LAYERS_CNT 1
#if ( TEX_LAYERS_CNT == 1 )
#define TERRAIN_TEX_FVF D3DFVF_TEX1
#elif ( TEX_LAYERS_CNT == 2 )
#define TERRAIN_TEX_FVF D3DFVF_TEX2
#elif ( TEX_LAYERS_CNT == 3 )
#define TERRAIN_TEX_FVF D3DFVF_TEX3
#endif

struct TerrainVertex
{
	D3DXVECTOR3		_p;
	D3DXVECTOR3		_n;
	D3DXVECTOR2		_t[TEX_LAYERS_CNT];
	static const DWORD FVF = D3DFVF_XYZ | D3DFVF_NORMAL | TERRAIN_TEX_FVF;
};

//////////////////////////////////////
// SceneManager.cpp
//////////////////////////////////////
struct DebugQuadTreeVertex {
	D3DXVECTOR3		_p;
	D3DCOLOR		_c;
	static const DWORD FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;
};

//////////////////////////////////////
// Skydome.cpp
//////////////////////////////////////
struct SkydomeVertex
{
	D3DXVECTOR3		_p;
	D3DXVECTOR2		_tex1;
	static const DWORD FVF = D3DFVF_XYZ | D3DFVF_TEX1;
};

//////////////////////////////////////
// BaseModel.cpp
//////////////////////////////////////
struct DebugAxesVertex
{
	D3DXVECTOR3		_p;
	D3DCOLOR		_c;
	static const DWORD FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;
};

struct SkeletonVertex
{
	D3DXVECTOR3		_p;
//	D3DCOLOR		_c;
	static const DWORD FVF = D3DFVF_XYZ;
};

//////////////////////////////////////
// RenderEngine.cpp
//////////////////////////////////////
struct RenderQuadVertex {
	D3DXVECTOR3		_p;
	static const DWORD FVF = D3DFVF_XYZ;
};
