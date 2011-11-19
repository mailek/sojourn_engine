#pragma once
//////////////////////////////////////////////////////////////////////////
// MeshManager.h - 2011 Matthew Alford
//////////////////////////////////////////////////////////////////////////

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
	typedef enum { eUnitSphere, eTeapot, eCenteredUnitABB, eScreenQuad } EGlobalMeshType;

private:
	LPDIRECT3DDEVICE9					m_device;
	MeshList							m_arrMeshes;
	MeshList							m_arrGlobalMeshes;
	D3DMATERIAL9						m_teapotMaterial;

	bool LoadGlobalMeshes(void);

public:
	void SetDevice(LPDIRECT3DDEVICE9 device);
	void GetMesh(EMeshType meshName, BaseModel **retMesh);
	void GetGlobalMesh(EGlobalMeshType meshName, BaseModel **retMesh);
	void Update( LPDIRECT3DDEVICE9 device, float elapsedMillis );
	
	inline MeshList* GetMeshList(void) {return &m_arrMeshes;}
	
};