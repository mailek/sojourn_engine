#include "StdAfx.h"
#include "MeshManager.h"
#include "Terrain.h"

//////////////////////////////////////////////////////////////////////////
// Setup Functions
//////////////////////////////////////////////////////////////////////////

CMeshManager::CMeshManager(void) : m_device(NULL)
{
}

CMeshManager::~CMeshManager(void)
{
	for(MeshList::iterator it = m_arrMeshes.begin(), _it = m_arrMeshes.end(); it != _it; it++)
		PTR_SAFEDELETE(*it);
}

void CMeshManager::SetDevice(LPDIRECT3DDEVICE9 device) 
{
	m_device=device;

	LoadGlobalMeshes();
}

bool CMeshManager::LoadGlobalMeshes(void)
{
	assert(m_device);

	/* Warning - these must maintain the same order as EGlobalMeshType */

	// Create the unit sphere
	BaseModel* sphere = new BaseModel();
	sphere->LoadCenteredUnitSphere(m_device);
	m_arrGlobalMeshes.push_back(sphere);

	// Create the teapot
	BaseModel* teapot = new BaseModel();
	teapot->LoadTeapot(m_device);
	m_arrGlobalMeshes.push_back(teapot);

	// Create Debug Centered ABB Cube
	BaseModel* abbCube = new BaseModel();
	abbCube->LoadCenteredUnitCube(m_device);
	m_arrGlobalMeshes.push_back(abbCube);

	// Create Screen Oriented Quad
	BaseModel* screenQuad = new BaseModel();
	screenQuad->LoadScreenOrientedQuad(m_device);
	m_arrGlobalMeshes.push_back(screenQuad);
	
	return true;
}

// Loads the meshes to be rendered
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
		tiny->Set3DPosition(D3DXVECTOR3(0.0f, 6.7f, 0.0f));
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
		animTiny->Set3DPosition(D3DXVECTOR3(0.0f, 0.0f, 0.0f));
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
		multiAnimTiny->Set3DPosition(D3DXVECTOR3(0.0f, 6.7f, 0.0f));
		m_arrMeshes.push_back(multiAnimTiny);
	} else return false;

	// Create well
	BaseModel* well = new BaseModel();
	if(well->LoadXMeshFromFile(RESOURCE_FOLDER"well.x", m_device))
	{
		strcpy_s(well->m_filename, "well.x");
		// NOTE: IN HERE GOES ANY CORRECTIONS NEEDED TO THE MODEL, THESE WILL BE APPLIED BEFORE ANY OTHER TRANFORMS

		//float scaleFactor(12.0f);
		float scaleFactor(1.0f);
		well->SetScale(D3DXVECTOR3(scaleFactor, scaleFactor, scaleFactor));
		well->Set3DPosition(D3DXVECTOR3(0.0f, 0.0f, 0.0f));
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
		cherryLow->Set3DPosition(D3DXVECTOR3(0.0f, -2.0f, 0.0f)); // sink the tree in the ground a little
		// trees need transparency for leaves
		cherryLow->SetTransparency(true);
		m_arrMeshes.push_back(cherryLow);
	} else return false;

	return true;

}

void CMeshManager::GetMesh(EMeshType meshName, BaseModel **retMesh)
{
	(*retMesh) = m_arrMeshes[meshName];
}

void CMeshManager::GetGlobalMesh(EGlobalMeshType meshName, BaseModel **retMesh)
{
	(*retMesh) = m_arrGlobalMeshes[meshName];
}

//////////////////////////////////////////////////////////////////////////
// Update Functions
//////////////////////////////////////////////////////////////////////////

void CMeshManager::Update( LPDIRECT3DDEVICE9 device, float elapsedMillis )
{
	for(MeshList::iterator it = m_arrMeshes.begin(), _it = m_arrMeshes.end(); it != _it; it++)
		(*it)->Update( device, elapsedMillis );

}