/********************************************************************
	created:	2012/04/23
	filename: 	LevelManager.cpp
	author:		Matthew Alford
	
	purpose:	
*********************************************************************/
#include "StdAfx.h"
#include "LevelManager.h"
#include "SceneManager.h"
#include "MeshManager.h"
#include "TerrainChunk.h"
#include "mathutil.h"
#include "RenderEngine.h"
#include "CollisionManager.h"

#define NUM_OF_TREES	50

//////////////////////////////////////////////////////////////////////////
// SceneObject Functions
//////////////////////////////////////////////////////////////////////////
/* NOTE: Need to refactor this out of levelmgr, as it shouldn't be here */

CLevelManager::SceneObject::SceneObject() : m_pMesh(NULL),
											m_lastFrame(0),
											m_bTransparent(false)
{
	collidableType = OBSTACLE;
}

void CLevelManager::SceneObject::Render( CRenderEngine &rndr )
{
	if(m_pMesh)
	{
		m_pMesh->SetRenderFunc(m_pMesh->RenderFuncs.lightAndTexture);
		m_pMesh->Render(rndr, m_transform.GetTransform(), rndr.GetShaderManager() );
	}
}

Sphere_PosRad CLevelManager::SceneObject::GetBoundingSphere()
{
	Sphere_PosRad ret;
	::ZeroMemory(&ret, sizeof(ret));
	if(m_pMesh) 
	{
		ret = m_pMesh->GetSphereBounds();
		Vec3_TransformCoord(&ret.pos, &ret.pos, &m_transform.GetTransform());

		/* use the average the the three scales */
		ret.radius *= (m_transform.GetScale().x + m_transform.GetScale().y + m_transform.GetScale().z)/3.0f;
	}

	return ret;
}

bool CLevelManager::SceneObject::IsTransparent()
{
	bool tp = false; 
	if(m_pMesh) 
		tp = m_pMesh->IsTransparent(); 
	else 
		tp = m_bTransparent; 
	
	return tp;
}

//////////////////////////////////////////////////////////////////////////
// Setup Functions
//////////////////////////////////////////////////////////////////////////

CLevelManager::CLevelManager(void)
{
}

CLevelManager::~CLevelManager(void)
{
	for( CDoubleLinkedList<SceneObject>::DoubleLinkedListItem* it = m_staticLevelObjects.first; it != NULL; it = it->next)
	{
		PTR_SAFEDELETE(it->item);
	}
}

bool CLevelManager::LoadDefaultLevel( CRenderEngine *renderEngine )
{
	CMeshManager& meshMgr = renderEngine->GetMeshManager();
	m_pSceneMgr = renderEngine->GetSceneManager();

	VERIFY(meshMgr.LoadMeshes());

	m_skyDome.LoadSkyDome(DRAW_DIST*0.8, 16, 16);

	/* load terrain */
	m_terrainMgr.Load("heightMap.raw", "groundTexture.dds", 128, 128, 20.0f, 0.9f, 30.0f);
	m_terrainMgr.SetYOffset(0.0f);

	/* load the level objects */

	SceneObject* obj;
	/* generate some trees and place randomly in level */
	for(int i = 0; i < NUM_OF_TREES; i++)
	{
		obj = new SceneObject();

		// generate some random x, z points, 
		ABB_MaxMin terrainBounds = m_terrainMgr.GetCurrentTerrain().CalculateBoundingBox();
		D3DXVECTOR3 diff = terrainBounds.max - terrainBounds.min;
		diff *= 0.5f;
		
		D3DXVECTOR3 newTreeLocation(fabs(diff.x)*(2.0f*frand() - 1.0f), 0, fabs(diff.z)*(2.0f*frand() - 1.0f));

		// ground clamp 
		newTreeLocation.y = m_terrainMgr.GetCurrentTerrain().GetTerrainHeightAtXZ(newTreeLocation.x, newTreeLocation.z);

		// set random position
		obj->SetLocalPosition(newTreeLocation);
		
		// random scale for trees
		const float fXZScaleFactor(2.0f), fYScaleFactor(0.7f);
		const float fXZRandomScale = 3.0f + frand()*fXZScaleFactor;
		const float fYRandomScale = 3.0f + frand()*fYScaleFactor;
		obj->SetScale(Vector_3(fXZRandomScale, fYRandomScale, fXZRandomScale));

		// random rotation
		obj->SetYRotationRadians(frand()*TWO_PI);
		
		//set the mesh to the cherry tree, and set transparency
		meshMgr.GetMesh(CMeshManager::eCherryTreeLow, &obj->m_pMesh);
		obj->m_bTransparent = true;

		//then add to scene mgr draw list
		m_staticLevelObjects.AddItemToEnd(obj);
	}

	/* well object on the origin */
	/*obj = new SceneObject();
	Vector_3 wellPos = Vector_3(0, 0, 0);
	wellPos.y = m_terrainMgr.GetCurrentTerrain().GetTerrainHeightAtXZ(wellPos.x,wellPos.y);
	obj->SetLocalPosition(wellPos);
	meshMgr.GetMesh(CMeshManager::eWell, &obj->m_pMesh);
	m_staticLevelObjects.AddItemToEnd(obj);*/

	/* setup lights */
	memset(m_lights, 0, sizeof(m_lights));
	AddDirLight(Vector_3(0.0f, -1.0f, 0.0f), Color_4(1.f, 1.0f, 1.0f, 1.0f));

	/* build the initial scene tree */
	RefreshSceneManager();

	return true;
}

void CLevelManager::RegisterStaticCollision(CCollisionManager* cm)
{
	for(CDoubleLinkedList<SceneObject>::DoubleLinkedListItem* it = m_staticLevelObjects.first; it != NULL; it = it->next)
	{
		cm->RegisterStaticCollidable(it->item);
	}

	/* set the collision manager on the terrain manager, so it can register new terrain as it is loaded */
	m_terrainMgr.SetCollisionMgr(cm);
}

void CLevelManager::AddDirLight(Vector_3 lightDir, Color_4 lightColor)
{
	LightObject* l = NULL;
	for( int i = 0; i < MAX_LIGHTS; i++)
	{
		if(!m_lights[i].used)
		{
			l = &m_lights[i];
			break;
		}
	}

	l->used				= true;
	l->isOn				= true;
	l->light.Type		= D3DLIGHT_DIRECTIONAL;
	l->light.Direction	= lightDir;
	l->light.Diffuse	= lightColor;
	l->light.Ambient	= lightColor * 0.5f;
	l->light.Specular	= lightColor * 0.3f;
}

void CLevelManager::OnTerrainGridChanged( CTerrainContainer* terrainGrid ) 
{
	RefreshSceneManager();
}

bool CLevelManager::HandleEvent( UINT eventId, void* data, UINT data_sz )
{
	switch( eventId )
	{
	case EVT_UPDATE:
		assert( data_sz == sizeof(float) );
		assert( data != NULL );
		Update( *(float*)data );
		break;
	case EVT_SETAVATAR:
		assert( data_sz == sizeof(SetAvatarEventArgType) );
		assert( data != NULL );
		m_terrainMgr.SetAvatar((SetAvatarEventArgType)data);
		m_skyDome.SetObjectToFollow((SetAvatarEventArgType)data);
		break;
	case EVT_TERRAIN_GRIDCHANGED:
		assert( data_sz == sizeof(TerrainGridChangedArgType) );
		assert( data != NULL );
		OnTerrainGridChanged((CTerrainContainer*)data);
		break;
	default:
		assert(false);
		break;
	}

	return true;
}

void CLevelManager::Update( float elapsedMillis )
{
	m_terrainMgr.Update( elapsedMillis );
}

void CLevelManager::RefreshSceneManager()
{
	/* rebuild the scene tree */
	CTerrainContainer terrain = m_terrainMgr.GetCurrentTerrain();
	m_pSceneMgr->BuildQuadTreeFromCurrentTerrain(terrain);

	/* add the terrain and skydome */
	for(int i = 0; i < 3; i++)
		for(int j = 0; j < 3; j++)
		{
			if(!terrain.m_terrainGrid[i][j])
				continue;

			m_pSceneMgr->AddNonclippableObjectToScene(terrain.m_terrainGrid[i][j]);
		}

	m_pSceneMgr->AddNonclippableObjectToScene(&m_skyDome);

	/* add static objects */
	for( CDoubleLinkedList<SceneObject>::DoubleLinkedListItem* it = m_staticLevelObjects.first; it != NULL; it = it->next)
	{
		m_pSceneMgr->AddRenderableObjectToScene(it->item);
	}
}