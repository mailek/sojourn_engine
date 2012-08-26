/********************************************************************
	created:	2012/04/16
	filename: 	SceneManager.cpp
	author:		Matthew Alford
	
	purpose:	
*********************************************************************/
#include "stdafx.h"

#include "SceneManager.h"
#include "ShaderManager.h"
#include "TerrainPatch.h"
#include "vertextypes.h"
#include "MeshManager.h"
#include "renderengine.h"

//////////////////////////////////////////////////////////////////////////
// Setup Functions
//////////////////////////////////////////////////////////////////////////

CSceneManager::CSceneManager(void) : m_debugMesh(NULL),
									m_clipStrategy(FRUSTUMCLIP)
{
	
}

CSceneManager::~CSceneManager(void)
{	
	
}

/************************************************
*   Name:   CSceneManager::Setup
*   Desc:   Initialize global class resources
************************************************/
void CSceneManager::Setup( CMeshManager& meshMgr )
{
	meshMgr.GetGlobalMesh(eCenteredUnitABB, &m_debugMesh);

}

//////////////////////////////////////////////////////////////////////////
// Update Functions
//////////////////////////////////////////////////////////////////////////

/************************************************
*   Name:   CSceneManager::UpdateDrawLists
*   Desc:   Build the FtoB and BtoF potentially
*           visible sets for the current camera
*           view.
************************************************/
void CSceneManager::UpdateDrawLists()
{
	/* if camera hasn't moved then no need to recalculate */
	D3DXMATRIX currentViewSpace = m_camera.GetViewMatrix();
	if(m_lastViewSpace == currentViewSpace)
		return; 

	/* empty the draw lists before filling out */
	m_opaqueList.clear();
	m_transparentList.clear();
	
	/* add any non-clippable objects to the lists */
	for(CDoubleLinkedList<IRenderable>::DoubleLinkedListItem* it = m_noClipList.first; it != NULL; it = it->next)
	{
		IRenderable* r = it->item;
		if( r->IsTransparent() )
		{
			ZSortableRenderable zsr;
			::ZeroMemory(&zsr, sizeof(zsr));
			zsr.p = r;
			D3DXVECTOR3 v(0,0,0);
			D3DXVec3TransformCoord(&v, &v, &D3DXMATRIX(r->GetWorldTransform() * currentViewSpace));
			zsr.viewSpaceZ = v.z;
			m_transparentList.push_back(zsr);
		}
		else
			m_opaqueList.push_back(r);
	}

	/* using the chosen strategy, build the draw lists from the quad tree */
	m_quadTree.NextFrame();
	D3DXVECTOR3 cameraPos = m_camera.Get3DPosition();
	switch(m_clipStrategy)
	{
	case BRUTEFORCE:
		{
			m_quadTree.GetAllRenderObjects( currentViewSpace, m_opaqueList, m_transparentList );
			break;
		}
	case FRUSTUMCLIP:
		{
			m_quadTree.GetRenderListsByCamera(m_camera, m_opaqueList, m_transparentList);
			break;
		}
	case CAMERAQUAD:
		{
			long cameraIndex = m_quadTree.FindLeafQuadByPoint(cameraPos);
			m_quadTree.AddObjectsToRenderListsByQuadIndex( &cameraIndex, 1, currentViewSpace, m_opaqueList, m_transparentList);
			break;
		}
	default:
		assert(false);
		break;
	}

	/* pre-sort the draw lists for the render engine according to transparency */
	//m_transparentList.sort();
	//m_transparentList.reverse();
	//m_transparentList.clear();

	//m_opaqueList.sort();
	//m_opaqueList.reverse();
		
	/* save camera position for next frame */
	m_lastViewSpace = currentViewSpace;
}

/************************************************
*   Name:   CSceneManager::GetOpaqueDrawListF2B
*   Desc:   Retrieve the PVS of opaque objects
*           sorted FtoB.
************************************************/
void CSceneManager::GetOpaqueDrawListF2B(std::list<IRenderable*> &list)
{
	UpdateDrawLists();
	list.clear();
	typedef std::list<IRenderable*> RenderList;

	m_opaqueList.sort();

	list = m_opaqueList;
}

/************************************************
*   Name:   CSceneManager::GetTransparentDrawListB2F
*   Desc:   Retrieve the PVS of transparent objects
*           sorted BtoF.
************************************************/
void CSceneManager::GetTransparentDrawListB2F(SceneMgrSortList &list)
{
	UpdateDrawLists();
	list.clear();

	m_transparentList.sort();
	m_transparentList.reverse();

	list = m_transparentList;
}

/************************************************
*   Name:   CSceneManager::GetAllObjectsDrawListB2F
*   Desc:   Retrieve all the PVS objects sorted BtoF.
************************************************/
void CSceneManager::GetAllObjectsDrawListB2F(SceneMgrSortList &list)
{
	UpdateDrawLists();
	list.clear();

	list = m_transparentList;
	for(SceneMgrRenderList::iterator it = m_opaqueList.begin(), _it = m_opaqueList.end(); it != _it; ++it)
	{
		ZSortableRenderable item;
		item.p = *it;

		D3DXVECTOR3 v(0,0,0);
		D3DXVec3TransformCoord(&v, &v, &D3DXMATRIX((*it)->GetWorldTransform() * m_camera.GetViewMatrix()));
		item.viewSpaceZ = v.z;

		list.push_back(item);
	}

	list.sort();
}

/************************************************
*   Name:   CSceneManager::AddNonclippableObjectToScene
*   Desc:   Add an object to the scene which will
*	        always be included in the PVS.
************************************************/
void CSceneManager::AddNonclippableObjectToScene(IRenderable* obj)
{
	assert(obj);
	m_noClipList.AddItemToEnd(obj);
}

/************************************************
*   Name:   CSceneManager::AddRenderableObjectToScene
*   Desc:   Add an object to the scene which may be
*           culled from the PVS if not in the camera
*           view.
************************************************/
void CSceneManager::AddRenderableObjectToScene(IRenderable* obj)
{
	assert(obj);
	m_quadTree.AddObjectToTree(obj);
}

//////////////////////////////////////////////////////////////////////////
// Render Functions
//////////////////////////////////////////////////////////////////////////

/************************************************
*   Name:   CSceneManager::Render
*   Desc:   Draw a debug grid to visualize the 
*           quad tree sectors.
************************************************/
void CSceneManager::Render( CRenderEngine &rndr )
{
	assert(m_debugMesh);
	CShaderManagerEx &shaderMgr = rndr.GetShaderManager();
	LPDIRECT3DDEVICE9 device = rndr.GetDevice();

	/* render the quad tree nodes for debug purposes */
	shaderMgr.PushCurrentShader();
	shaderMgr.SetEffect(EFFECT_DEBUGDRAW);
	shaderMgr.SetTechnique("FixedColor");
	shaderMgr.SetViewProjectionEx( m_camera.GetViewMatrix(), m_camera.GetProjectionMatrix() );

	device->SetFVF(m_debugMesh->GetFVF());
	device->SetTexture(0, 0);

	LPDIRECT3DINDEXBUFFER9 indices;
	LPDIRECT3DVERTEXBUFFER9 vertices;
	m_debugMesh->GetIndexBuffer( &indices );
	m_debugMesh->GetVertexBuffer( &vertices );
	device->SetIndices( indices );
	device->SetStreamSource(0, vertices, 0, m_debugMesh->GetVertexSizeInBytes());

	// render state
	dxEnableZWrite();
	dxEnableZTest();
	dxEnableAlphaBlend( false );
	dxEnableAlphaTest( false ); 
	dxEnableDithering();
	dxCullMode( D3DCULL_NONE );

	// stencil setup
	dxEnableStencilTest();
	dxStencilOp( D3DSTENCILOP_REPLACE, D3DSTENCILOP_KEEP, D3DSTENCILOP_KEEP );
	dxStencilFunc( D3DCMP_EQUAL, 0, 0 );

	// background
	shaderMgr.SetDrawColorEx(D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f));
	int num = shaderMgr.BeginEffect();
	for(int i=0; i<num; i++)
	{
		shaderMgr.Pass(i);
		for(int i = 0, j = m_quadTree.GetQuadTreeSize(); i < j; i++)
		{
			shaderMgr.SetWorldTransformEx( m_quadTree.GetWorldTransformByQuadIndex(i) );
			shaderMgr.CommitEffectParams();
			device->DrawPrimitive(D3DPT_LINESTRIP, 0, m_debugMesh->GetNumVertices());		
		}
		shaderMgr.FinishPass();
	}

	shaderMgr.FinishEffect();

	// decal
	dxEnableZWrite(false);
	dxEnableZTest(false);

	// find the leaf quad the camera is currently within, and display it highlighted with red
	int cameraQuadIndex = m_quadTree.FindLeafQuadByPoint(m_camera.Get3DPosition());
	shaderMgr.SetWorldTransformEx( m_quadTree.GetWorldTransformByQuadIndex(cameraQuadIndex) );
	shaderMgr.SetDrawColorEx( D3DXCOLOR(1.0f, 0.2f, 0.2f, 1.0f) );

	num = shaderMgr.BeginEffect();
	for(int i=0; i<num; i++)
	{
		shaderMgr.Pass(i);
		device->DrawPrimitive(D3DPT_LINESTRIP, 0, m_debugMesh->GetNumVertices());
		shaderMgr.FinishPass();
	}

	shaderMgr.FinishEffect();

	// stencil tear down
	dxEnableZWrite();
	dxEnableZTest();
	dxEnableStencilTest(false);
	dxEnableDithering(false);

	shaderMgr.PopCurrentShader();
	COM_SAFERELEASE(indices);
	COM_SAFERELEASE(vertices);
}

/************************************************
*   Name:   CSceneManager::SetNextClipStrategy
*   Desc:   Set the PVS determining algorithm.
************************************************/
void CSceneManager::SetNextClipStrategy(int strategy/* = -1*/)
{
	if(strategy >= FIRST_STRATEGY) 
		m_clipStrategy = strategy; 
	else 
	{ 
		m_clipStrategy++; 
		m_clipStrategy = m_clipStrategy >= STRATEGY_CNT ? FIRST_STRATEGY : m_clipStrategy; 
	} 
}

void CSceneManager::BuildQuadTreeFromCurrentTerrain( CTerrainContainer &terrain )
{
	/* Create the quad tree for frustum culling acceleration */
	ABB_MaxMin abb = terrain.CalculateBoundingBox();
	CSceneQuadTree::GCQTDefinition qtdef;
	qtdef.branchHeight = 500.0f;
	qtdef.leafHeight = 50.0f;
	qtdef.quadTreeDepth = 5;
	m_quadTree.Setup( abb, terrain, qtdef );
}
