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

	// Create the teapot
	BaseModel* teapot = new BaseModel();
	if(teapot->LoadTeapot(m_device))
		m_arrGlobalMeshes.push_back(teapot);
	else return false;

	// Create Debug Centered ABB Cube
	BaseModel* abbCube = new BaseModel();
	if(abbCube->LoadCenteredCube(m_device))
		m_arrGlobalMeshes.push_back(abbCube);
	else return false;

	// Create Screen Oriented Quad
	BaseModel* screenQuad = new BaseModel();
	if(screenQuad->LoadScreenOrientedQuad(m_device))
		m_arrGlobalMeshes.push_back(screenQuad);
	else return false;

	return true;
}

// Loads the meshes to be rendered
bool CMeshManager::LoadMeshes(void)
{
	assert(m_device);

	// IMPORTANT: The order these are created and put in m_arrMeshes matters here, since using an enum to index

	// Create tiny
	BaseModel* tiny = new BaseModel();
	if(tiny->LoadXMeshFromFile("Resources\\tiny.x", m_device))
	{
		// NOTE: IN HERE GOES ANY CORRECTIONS NEEDED TO THE MODEL, THESE WILL BE APPLIED BEFORE ANY OTHER TRANFORMS

		float scaleFactor(0.025f);
		tiny->SetScale(D3DXVECTOR3(scaleFactor, scaleFactor, scaleFactor)); // tiny model is way too big, scale down
		tiny->SetXRotationRadians(-D3DX_PI * 0.5f); // flip to standing orientation
		tiny->Set3DPosition(D3DXVECTOR3(0.0f, 6.7f, 0.0f));
		m_arrMeshes.push_back(tiny);
	} else return false;

	// Create animated tiny
	BaseModel* animTiny = new BaseModel();
	if(animTiny->LoadXMeshHierarchyFromFile("Resources\\tiny_2.x", m_device))
	{
		// NOTE: IN HERE GOES ANY CORRECTIONS NEEDED TO THE MODEL, THESE WILL BE APPLIED BEFORE ANY OTHER TRANFORMS

		float scaleFactor(0.025f);
		animTiny->SetScale(D3DXVECTOR3(scaleFactor, scaleFactor, scaleFactor)); // tiny model is way too big, scale down
		animTiny->SetXRotationRadians(-D3DX_PI * 0.5f); 
		animTiny->Set3DPosition(D3DXVECTOR3(0.0f, 0.0f, 0.0f));
		m_arrMeshes.push_back(animTiny);
	} else return false;

	BaseModel* multiAnimTiny = new BaseModel();
	if(multiAnimTiny->LoadXMeshHierarchyFromFile("Resources\\brokentiny.x", m_device))
	{
		// NOTE: IN HERE GOES ANY CORRECTIONS NEEDED TO THE MODEL, THESE WILL BE APPLIED BEFORE ANY OTHER TRANFORMS

		float scaleFactor(0.025f);
		multiAnimTiny->SetScale(D3DXVECTOR3(scaleFactor, scaleFactor, scaleFactor)); // tiny model is way too big, scale down
		multiAnimTiny->SetYRotationRadians(-D3DX_PI); // flip to standing orientation;
		multiAnimTiny->Set3DPosition(D3DXVECTOR3(0.0f, 6.7f, 0.0f));
		m_arrMeshes.push_back(multiAnimTiny);
	} else return false;

	// Create well
	BaseModel* well = new BaseModel();
	if(well->LoadXMeshFromFile("Resources\\well.x", m_device))
	{
		// NOTE: IN HERE GOES ANY CORRECTIONS NEEDED TO THE MODEL, THESE WILL BE APPLIED BEFORE ANY OTHER TRANFORMS

		float scaleFactor(12.0f);
		well->SetScale(D3DXVECTOR3(scaleFactor, scaleFactor, scaleFactor));
		well->Set3DPosition(D3DXVECTOR3(0.0f, 0.0f, 0.0f));
		m_arrMeshes.push_back(well);
	} else return false;

	// Create cherry tree low
	BaseModel* cherryLow = new BaseModel();
	if(cherryLow->LoadXMeshFromFile("Resources\\cherryTreeLow.x", m_device))
	{
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