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
#include "TerrainPatch.h"
#include "RenderEngine.h"
#include "CollisionManager.h"

#define NUM_OF_TREES	10	

//////////////////////////////////////////////////////////////////////////
// LevelObject Functions
//////////////////////////////////////////////////////////////////////////
/* NOTE: Need to refactor this out of levelmgr, as it shouldn't be here */

CLevelManager::LevelObject::LevelObject() : m_pMesh(NULL),
											m_lastFrame(0),
											m_bTransparent(false)
{
	collidableType = OBSTACLE;
}

void CLevelManager::LevelObject::Render( CRenderEngine &rndr )
{
	if(!m_pMesh)
		return;
	
	m_pMesh->SetRenderFunc(m_pMesh->RenderFuncs.lightAndTexture);
	m_pMesh->Render(rndr, m_transform.GetTransform(), rndr.GetShaderManager() );
}

Sphere_PosRad CLevelManager::LevelObject::GetBoundingSphere()
{
	Sphere_PosRad ret;
	if(!m_pMesh)
	{
		::ZeroMemory(&ret, sizeof(ret));
		return ret;
	}

	ret = m_pMesh->GetSphereBounds();
	Vec3_TransformCoord(&ret.pos, &ret.pos, &m_transform.GetTransform());

	/* use the average the the three scales */
	Vector_3 s = m_transform.GetScale();
	ret.radius *= (s.x + s.y + s.z)/3.0f;

	return ret;
}

bool CLevelManager::LevelObject::IsTransparent()
{
	return( m_pMesh ? m_pMesh->IsTransparent() : m_bTransparent );
}

//////////////////////////////////////////////////////////////////////////
// Setup Functions
//////////////////////////////////////////////////////////////////////////

CLevelManager::CLevelManager(void) : m_pSceneMgr(NULL)
{
	::ZeroMemory(&m_levelState.lights, sizeof(m_levelState.lights));
}

CLevelManager::~CLevelManager(void)
{
	for( CDoubleLinkedList<LevelObject>::DoubleLinkedListItem* it = m_levelState.staticLevelObjects.first; it != NULL; it = it->next)
	{
		ptr_safedelete(it->item);
	}
}

void CLevelManager::LoadDefaultLevel( CRenderEngine *renderEngine )
{
	CMeshManager& meshMgr = renderEngine->GetMeshManager();
	m_pSceneMgr = renderEngine->GetSceneManager();

	VERIFY(meshMgr.LoadAllResidentMeshes());

	m_levelState.skyDome.LoadSkyDome(DRAW_DIST*0.8, 16, 16);

	/* load terrain */
	m_levelState.terrain.Setup("heightmap641x641-4.raw", 641, 641);
	m_levelState.terrain.SetYOffset(0.0f);

	/* load the level objects */

	//LevelObject* obj;
	/* generate some trees and place randomly in level */
	//for(int i = 0; i < NUM_OF_TREES; i++)
	//{
	//	obj = new LevelObject();

	//	// generate some random x, z points, 
	//	ABB_MaxMin terrainBounds = m_terrain.GetCurrentTerrain().CalculateBoundingBox();
	//	D3DXVECTOR3 diff = terrainBounds.max - terrainBounds.min;
	//	diff *= 0.5f;
	//	
	//	D3DXVECTOR3 newTreeLocation(fabs(diff.x)*(2.0f*frand() - 1.0f), 0, fabs(diff.z)*(2.0f*frand() - 1.0f));

	//	// ground clamp 
	//	newTreeLocation.y = m_terrain.GetCurrentTerrain().GetTerrainHeightAtXZ(newTreeLocation.x, newTreeLocation.z);

	//	// set random position
	//	obj->SetLocalPosition(newTreeLocation);
	//	
	//	// random scale for trees
	//	const float fXZScaleFactor(2.0f), fYScaleFactor(0.7f);
	//	const float fXZRandomScale = 3.0f + frand()*fXZScaleFactor;
	//	const float fYRandomScale = 3.0f + frand()*fYScaleFactor;
	//	obj->SetScale(Vector_3(fXZRandomScale, fYRandomScale, fXZRandomScale));

	//	// random rotation
	//	obj->SetYRotationRadians(frand()*TWO_PI);
	//	
	//	//set the mesh to the cherry tree, and set transparency
	//	meshMgr.GetMesh(CMeshManager::eCherryTreeLow, &obj->m_pMesh);
	//	obj->m_bTransparent = true;

	//	//then add to scene mgr draw list
	//	m_staticLevelObjects.AddItemToEnd(obj);
	//}

	/* well object on the origin */
	/*obj = new LevelObject();
	Vector_3 wellPos = Vector_3(0, 0, 0);
	wellPos.y = m_terrain.GetCurrentTerrain().GetTerrainHeightAtXZ(wellPos.x,wellPos.y);
	obj->SetLocalPosition(wellPos);
	meshMgr.GetMesh(CMeshManager::eWell, &obj->m_pMesh);
	m_staticLevelObjects.AddItemToEnd(obj);*/

	/* setup lights */
	memset(m_levelState.lights, 0, sizeof(m_levelState.lights));
	AddDirLight(Vector_3(0.0f, -1.0f, 0.0f), Color_4(1.f, 1.0f, 1.0f, 1.0f));

	return;
}

void CLevelManager::RegisterStaticCollision(CCollisionManager* cm)
{
	for(CDoubleLinkedList<LevelObject>::DoubleLinkedListItem* it = m_levelState.staticLevelObjects.first; it != NULL; it = it->next)
	{
		cm->RegisterStaticCollidable(it->item);
	}

	/* set the collision manager on the terrain manager, so it can register new terrain as it is loaded */
	m_levelState.terrain.SetCollisionMgr(cm);
}

void CLevelManager::AddDirLight(Vector_3 lightDir, Color_4 lightColor)
{
	LightObject* lt = NULL;
	for( int i = 0; i < MAX_LIGHTS; i++)
	{
		if(!m_levelState.lights[i].used)
		{
			lt = &m_levelState.lights[i];
			break;
		}
	}

	if(lt)
	{
		lt->used			= true;
		lt->isOn			= true;
		lt->light.Type		= D3DLIGHT_DIRECTIONAL;
		lt->light.Direction	= lightDir;
		lt->light.Diffuse	= lightColor;
		lt->light.Ambient	= lightColor * 0.5f;
		lt->light.Specular	= lightColor * 0.3f;
	}
}

void CLevelManager::OnTerrainGridChanged( CTerrainContainer* terrainGrid ) 
{
	RefreshSceneManager();
}

bool CLevelManager::HandleEvent( UINT eventId, void* data, UINT data_sz )
{
	bool handled = false;
	switch( eventId )
	{
	case EVT_UPDATE:
		assert( data_sz == sizeof(float) );
		assert( data != NULL );
		Update( *(float*)data );
		handled = true;
		break;
	case EVT_SETAVATAR:
		{
		DASSERT(data && data_sz == sizeof(SetAvatarEventArgType));
		SetAvatarEventArgType args = __sc(SetAvatarEventArgType, data);
		m_levelState.terrain.SetAvatar(args);
		m_levelState.skyDome.SetObjectToFollow((SetAvatarEventArgType)data);
		RefreshSceneManager();
		handled = true;
		break;
		}
	case EVT_TERRAIN_GRIDCHANGED:
		assert( data_sz == sizeof(TerrainGridChangedArgType) );
		assert( data != NULL );
		OnTerrainGridChanged(reinterpret_cast<CTerrainContainer*>(data));
		handled = true;
		break;
	default:
		assert(false);
		break;
	}

	return handled;
}

void CLevelManager::Update( float elapsedMillis )
{
	m_levelState.terrain.Update( elapsedMillis );
}

void CLevelManager::RefreshSceneManager()
{
	/* rebuild the scene tree */
	CTerrainContainer terrain = m_levelState.terrain.GetCurrentTerrain();
	m_pSceneMgr->BuildQuadTreeFromCurrentTerrain(terrain);

	/* add the terrain and skydome */
	for(int i = 0; i < 3; i++)
		for(int j = 0; j < 3; j++)
		{
			if(!terrain.m_terrainGrid[i][j])
				continue;

			DASSERT(terrain.m_terrainGrid[i][j]->IsRenderMeshLoaded());
			m_pSceneMgr->AddNonclippableObjectToScene(terrain.m_terrainGrid[i][j]);
		}

	m_pSceneMgr->AddNonclippableObjectToScene(&m_levelState.skyDome);

	/* add static objects */
	for( CDoubleLinkedList<LevelObject>::DoubleLinkedListItem* it = m_levelState.staticLevelObjects.first; it != NULL; it = it->next)
	{
		m_pSceneMgr->AddRenderableObjectToScene(it->item);
	}
}