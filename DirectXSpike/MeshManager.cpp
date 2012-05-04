/********************************************************************
	created:	2012/04/16
	filename: 	MeshManager.cpp
	author:		Matthew Alford
	
	purpose:	
*********************************************************************/
#include "StdAfx.h"
#include "MeshManager.h"
#include "TerrainChunk.h"

/*=================================================
	MeshManager Class
=================================================*/

CMeshManager::CMeshManager(void) : m_device(NULL)
{
}

CMeshManager::~CMeshManager(void)
{
	for(MeshList::iterator it = m_arrMeshes.begin(), _it = m_arrMeshes.end(); it != _it; it++)
		PTR_SAFEDELETE(*it);
}

/*=================================================
	Setup Functions
=================================================*/

/************************************************
*   Name:   CMeshManager::SetDevice
*   Desc:   Set the DirectX device and load the
*           the debug meshes.
************************************************/
void CMeshManager::SetDevice(LPDIRECT3DDEVICE9 device) 
{
	m_device=device;

	LoadGlobalMeshes();
}

/************************************************
*   Name:   CMeshManager::LoadGlobalMeshes
*   Desc:   Loads the palette of debug meshes.
************************************************/
bool CMeshManager::LoadGlobalMeshes(void)
{
	BaseModel *model;

	for(int i = 0; i < globalMeshCnt; i++)
	{
		model = new BaseModel();

		switch(i)
		{
		case eUnitSphere:
			model->LoadCenteredUnitSphere();
			break;
		case eUnitCylinder:
			model->LoadCenteredUnitCylinder();
			break;
		case eTeapot:
			model->LoadTeapot();
			break;
		case eCenteredUnitABB:
			model->LoadCenteredUnitCube();
			break;
		case eScreenQuad:
			model->LoadScreenOrientedQuad();
			break;
		default:
			assert(false);
		}

		m_arrGlobalMeshes[i] = model;
	}
	
	return true;
}

/************************************************
*   Name:   CMeshManager::LoadMeshes
*   Desc:   Loads the level's models.  Currently
*           all meshes are loaded statically.
************************************************/
bool CMeshManager::LoadMeshes(void)
{
	assert(m_device);

	// IMPORTANT: The order these are created and put in m_arrMeshes matters here, since using an enum to index

	// Create tiny
	BaseModel* tiny = new BaseModel();
	if(tiny->LoadXMeshFromFile(RESOURCE_FOLDER"tiny.x", m_device))
	{
		strcpy_s(tiny->m_filename, "tiny.x");

		// NOTE: IN HERE GOES ANY CORRECTIONS NEEDED TO THE MODEL, THESE WILL BE APPLIED BEFORE ANY OTHER TRANFORMS
		float scaleFactor(0.025f);
		tiny->SetScale(D3DXVECTOR3(scaleFactor, scaleFactor, scaleFactor)); // tiny model is way too big, scale down
		tiny->SetXRotationRadians(-D3DX_PI * 0.5f); // flip to standing orientation
		tiny->SetLocalPosition(D3DXVECTOR3(0.0f, 6.7f, 0.0f));
		m_arrMeshes.push_back(tiny);
	} else return false;

	// Create animated tiny
	BaseModel* animTiny = new BaseModel();
	if(animTiny->LoadXMeshHierarchyFromFile(RESOURCE_FOLDER"tiny_2.x", m_device))
	{
		strcpy_s(animTiny->m_filename, "tiny_2.x");
		// NOTE: IN HERE GOES ANY CORRECTIONS NEEDED TO THE MODEL, THESE WILL BE APPLIED BEFORE ANY OTHER TRANFORMS

		float scaleFactor(0.025f);
		animTiny->SetScale(D3DXVECTOR3(scaleFactor, scaleFactor, scaleFactor)); // tiny model is way too big, scale down
		animTiny->SetXRotationRadians(-D3DX_PI * 0.5f); 
		animTiny->SetLocalPosition(D3DXVECTOR3(0.0f, 0.0f, 0.0f));
		m_arrMeshes.push_back(animTiny);
	} else return false;

	BaseModel* multiAnimTiny = new BaseModel();
	if(multiAnimTiny->LoadXMeshHierarchyFromFile(RESOURCE_FOLDER"brokentiny.x", m_device))
	{
		strcpy_s(multiAnimTiny->m_filename, "brokentiny.x");
		// NOTE: IN HERE GOES ANY CORRECTIONS NEEDED TO THE MODEL, THESE WILL BE APPLIED BEFORE ANY OTHER TRANFORMS

		float scaleFactor(0.025f);
		multiAnimTiny->SetScale(D3DXVECTOR3(scaleFactor, scaleFactor, scaleFactor)); // tiny model is way too big, scale down
		multiAnimTiny->SetYRotationRadians(-D3DX_PI); // flip to standing orientation;
		multiAnimTiny->SetLocalPosition(D3DXVECTOR3(0.0f, 6.7f, 0.0f));
		m_arrMeshes.push_back(multiAnimTiny);
	} else return false;

	// Create well
	BaseModel* well = new BaseModel();
	if(well->LoadXMeshFromFile(RESOURCE_FOLDER"well.x", m_device))
	{
		strcpy_s(well->m_filename, "well.x");
		// NOTE: IN HERE GOES ANY CORRECTIONS NEEDED TO THE MODEL, THESE WILL BE APPLIED BEFORE ANY OTHER TRANFORMS

		float scaleFactor(8.0f);
		well->SetScale(D3DXVECTOR3(scaleFactor, scaleFactor, scaleFactor));
		well->SetLocalPosition(D3DXVECTOR3(0.0f, 0.0f, 0.0f));
		m_arrMeshes.push_back(well);
	} else return false;

	// Create cherry tree low
	BaseModel* cherryLow = new BaseModel();
	if(cherryLow->LoadXMeshFromFile(RESOURCE_FOLDER"cherryTreeLow.x", m_device))
	{
		strcpy_s(cherryLow->m_filename, "cherryTreeLow.x");
		// NOTE: IN HERE GOES ANY CORRECTIONS NEEDED TO THE MODEL, THESE WILL BE APPLIED BEFORE ANY OTHER TRANFORMS

		float scaleFactor(4.0f);
		cherryLow->SetScale(D3DXVECTOR3(scaleFactor, scaleFactor, scaleFactor));
		cherryLow->SetLocalPosition(D3DXVECTOR3(0.0f, -2.0f, 0.0f)); // sink the tree in the ground a little
		// trees need transparency for leaves
		cherryLow->SetTransparency(true);
		m_arrMeshes.push_back(cherryLow);
	} else return false;

	return true;

}

/************************************************
*   Name:   CMeshManager::GetMesh
*   Desc:   Retrieve a loaded model.
************************************************/
void CMeshManager::GetMesh(EMeshType meshName, BaseModel **retMesh)
{
	(*retMesh) = m_arrMeshes[meshName];
}

/************************************************
*   Name:   CMeshManager::GetGlobalMesh
*   Desc:   Retrieve a debug model.
************************************************/
void CMeshManager::GetGlobalMesh(EGlobalMeshType meshName, BaseModel **retMesh)
{
	(*retMesh) = m_arrGlobalMeshes[meshName];
}

/*=================================================
	Update Functions
=================================================*/

/************************************************
*   Name:   CMeshManager::Update
*   Desc:   Send the animated models the update 
*           signal.
************************************************/
void CMeshManager::Update( float elapsedMillis )
{
	for(MeshList::iterator it = m_arrMeshes.begin(), _it = m_arrMeshes.end(); it != _it; it++)
		(*it)->Update( elapsedMillis );
}