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
class CTerrainPatch;

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
	EGlobalMeshTypeCnt
} EGlobalMeshType;

typedef enum _EResidentMeshID
{ 
	eTinyX, eAnimTiny, eMultiAnimTiny, eWell, eCherryTreeLow 
} EResidentMeshID;

//////////////////////////////////////
// Class Definition
//////////////////////////////////////
class CMeshManager
{
public:
	CMeshManager(void);
	~CMeshManager(void);

private:
	LPDIRECT3DDEVICE9					m_device;
	MeshList							m_arrMeshes;
	BaseModel*							m_arrGlobalMeshes[EGlobalMeshTypeCnt];
	D3DMATERIAL9						m_teapotMaterial;

	bool LoadGlobalMeshes(void);

public:
	void SetDevice(LPDIRECT3DDEVICE9 device);
	void GetMesh(EResidentMeshID meshName, BaseModel **retMesh);
	void GetGlobalMesh(EGlobalMeshType meshName, BaseModel **retMesh);
	void RunAnimations( float elapsedMillis );
	bool LoadAllResidentMeshes(void);// Loads the meshes to be rendered
	
	inline MeshList* GetMeshList(void) {return &m_arrMeshes;}
	
};