#pragma once
/********************************************************************
	created:	2012/04/16
	filename: 	MeshManager.h
	author:		Matthew Alford
	
	purpose:	
*********************************************************************/

//////////////////////////////////////
// Includes
//////////////////////////////////////
#include "BaseModel.h"

//////////////////////////////////////
// Forward Declarations
//////////////////////////////////////
class CTerrain;

//////////////////////////////////////
// Type Definitions
//////////////////////////////////////
typedef std::vector<BaseModel*> MeshList;
typedef MeshList::iterator MeshListIterator;

typedef enum _EGlobalMeshType 
{ 
	eUnitSphere, 
	eUnitCylinder, 
	eTeapot, 
	eCenteredUnitABB, 
	eScreenQuad,
	globalMeshCnt
} EGlobalMeshType;

//////////////////////////////////////
// Class Definition
//////////////////////////////////////
class CMeshManager
{
public:
	CMeshManager(void);
	~CMeshManager(void);
	// Loads the meshes to be rendered
	bool LoadMeshes(void);

	typedef enum { eTinyX, eAnimTiny, eMultiAnimTiny, eWell, eCherryTreeLow } EMeshType;

private:
	LPDIRECT3DDEVICE9					m_device;
	MeshList							m_arrMeshes;
	BaseModel*							m_arrGlobalMeshes[globalMeshCnt];
	D3DMATERIAL9						m_teapotMaterial;

	bool LoadGlobalMeshes(void);

public:
	void SetDevice(LPDIRECT3DDEVICE9 device);
	void GetMesh(EMeshType meshName, BaseModel **retMesh);
	void GetGlobalMesh(EGlobalMeshType meshName, BaseModel **retMesh);
	void Update( LPDIRECT3DDEVICE9 device, float elapsedMillis );
	
	inline MeshList* GetMeshList(void) {return &m_arrMeshes;}
	
};