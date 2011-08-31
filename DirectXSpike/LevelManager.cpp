#include "StdAfx.h"
#include "LevelManager.h"
#include "SceneManager.h"
#include "MeshManager.h"
#include "Terrain.h"
#include "mathutil.h"
#include "RenderEngine.h"

#define NUM_OF_TREES	50

//////////////////////////////////////////////////////////////////////////
// SceneObject Functions
//////////////////////////////////////////////////////////////////////////

CLevelManager::SceneObject::SceneObject() : m_pMesh(NULL)
{
	::ZeroMemory(&m_boundSphere, sizeof(m_boundSphere));
}

void CLevelManager::SceneObject::Render( CRenderEngine &rndr )
{
	if(m_pMesh) 
		m_pMesh->Render(rndr.GetDevice(), m_transform, rndr.GetShaderManager() );
}

//////////////////////////////////////////////////////////////////////////
// Setup Functions
//////////////////////////////////////////////////////////////////////////

CLevelManager::CLevelManager(void)
{
}

CLevelManager::~CLevelManager(void)
{
	for(std::list<SceneObject*>::iterator it = m_staticLevelObjects.begin(), _it = m_staticLevelObjects.end(); it != _it; it++)
		PTR_SAFEDELETE(*it);
}

bool CLevelManager::LoadDefaultLevel(CRenderEngine *renderEngine, CTerrain **retTerrain, CSkybox **retSkybox)
{
	CMeshManager& meshMgr = renderEngine->GetMeshManager();
	CSceneManager& sceneMgr = renderEngine->GetSceneManager();
	LPDIRECT3DDEVICE9 device = renderEngine->GetDevice();

	if(!meshMgr.LoadMeshes())
		return false;

	// create skybox
	m_pSkyDome = new CSkybox();
	if(retSkybox)
		*retSkybox = m_pSkyDome;
	
	if(!m_pSkyDome->LoadSkyDome(device, DRAW_DIST*0.8, 16, 16))
		return false;

	// create terrain
	m_pTerrain = new CTerrain();
	if(retTerrain)
		*retTerrain = m_pTerrain;

	m_pTerrain->SetYOffset(0.0f);
	if(!m_pTerrain->LoadTerrain("Resources\\heightMap.raw", "Resources\\groundTexture.dds", 128, 128, 20.0f, 0.9f, 30.0f, device ))
		return false;

	renderEngine->GetSceneManager().Setup(device, *m_pTerrain, meshMgr);

	// add the terrain and skydome
	sceneMgr.AddNonclippableObjectToScene(m_pTerrain);
	sceneMgr.AddNonclippableObjectToScene(m_pSkyDome);

	SceneObject* obj;
	// generate some trees and place randomly in level
	for(int i = 0; i < NUM_OF_TREES; i++)
	{
		obj = new SceneObject();

		// generate some random x, z points, 
		ABB_MaxMin terrainBounds = m_pTerrain->CalculateBoundingBox();
		D3DXVECTOR3 diff = terrainBounds.max - terrainBounds.min;
		diff *= 0.5f;
		
		D3DXVECTOR3 newTreeLocation(fabs(diff.x)*(2.0f*frand() - 1.0f), 0, fabs(diff.z)*(2.0f*frand() - 1.0f));
		
		//ground clamp 
		newTreeLocation.y = m_pTerrain->GetTerrainHeightAtXZ(newTreeLocation.x, newTreeLocation.z);
		
		// random scale for trees
		D3DXMATRIX randomScale;
		const float fXZScaleFactor(2.0f), fYScaleFactor(0.7f);
		float fXZRandomScale = 3.0f + frand()*fXZScaleFactor;
		float fYRandomScale = 3.0f + frand()*fYScaleFactor;
		D3DXMatrixScaling(&randomScale, fXZRandomScale, fYRandomScale, fXZRandomScale);

		// random rotation
		D3DXMATRIX randomRotation;
		D3DXMatrixRotationY(&randomRotation, frand()*2*PI);
		
		// set the world transform
		D3DXMatrixIdentity(&obj->m_transform);
		D3DXMATRIX translate;
		D3DXMatrixTranslation(&translate, newTreeLocation.x, newTreeLocation.y, newTreeLocation.z);
		obj->m_transform *= randomScale * randomRotation * translate;
		obj->m_bTransparent = true;

		//set the mesh to the cherry tree 
		meshMgr.GetMesh(CMeshManager::eCherryTreeLow, &obj->m_pMesh);
		Sphere_PosRad sphere = obj->m_pMesh->GetSphereBounds();
				
		// calculate the world position of the bounding sphere by applying the object world transform, using only the highest scale of the three axes
		D3DXMATRIX sphereTransform = obj->m_transform;
		float radiusScale = max(sphereTransform._11, max(sphereTransform._22, sphereTransform._33));
				
		D3DXVec3TransformCoord(&obj->m_boundSphere.pos, &sphere.pos, &sphereTransform);
		obj->m_boundSphere.radius = sphere.radius*3*radiusScale; // HACK HACK HACK : this coefficient of 3 is a magic radius multiplier because sphere calc is giving bad radius FIX ASAP
		
		//then add to scene mgr draw list
		m_staticLevelObjects.push_back(obj);
		sceneMgr.AddRenderableObjectToScene(reinterpret_cast<IRenderable*>(obj));
	}

	// well object on the origin
	obj = new SceneObject();
	D3DXMatrixTranslation(&obj->m_transform, 100, m_pTerrain->GetTerrainHeightAtXZ(100,100), 100);
	meshMgr.GetMesh(CMeshManager::eWell, &obj->m_pMesh);
	obj->m_boundSphere = obj->m_pMesh->GetSphereBounds();
	D3DXVec3TransformCoord(&obj->m_boundSphere.pos, &obj->m_boundSphere.pos, &obj->m_transform);
	m_staticLevelObjects.push_back(obj);
	sceneMgr.AddRenderableObjectToScene(reinterpret_cast<IRenderable*>(obj));

	return true;
}