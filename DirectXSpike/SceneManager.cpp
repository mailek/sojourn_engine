#include "stdafx.h"

#include "SceneManager.h"
#include "ShaderManager.h"
#include "Terrain.h"
#include "MathUtil.h"
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

void CSceneManager::Setup(LPDIRECT3DDEVICE9 device, CTerrain& terrain, CMeshManager& meshMgr)
{
	meshMgr.GetGlobalMesh(CMeshManager::eCenteredUnitABB, &m_debugMesh);

	// Create the quad tree for frustum culling acceleration
	ABB_MaxMin abb = terrain.CalculateBoundingBox();
	QuadTree_GroundClamped<IRenderable, CTerrain>::GCQTDefinition qtdef;
	qtdef.branchHeight = 500.0f;
	qtdef.leafHeight = 50.0f;
	qtdef.quadTreeDepth = 5;
	m_quadTree.Setup( abb, terrain, qtdef );

}

//////////////////////////////////////////////////////////////////////////
// Update Functions
//////////////////////////////////////////////////////////////////////////

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

void CSceneManager::GetOpaqueDrawListF2B(std::list<IRenderable*> &list)
{
	UpdateDrawLists();
	list.clear();
	typedef std::list<IRenderable*> RenderList;

	m_opaqueList.sort();

	list = m_opaqueList;
}

void CSceneManager::GetTransparentDrawListB2F(SceneMgrSortList &list)
{
	UpdateDrawLists();
	list.clear();

	m_transparentList.sort();
	m_transparentList.reverse();

	list = m_transparentList;
}

void CSceneManager::AddNonclippableObjectToScene(IRenderable* obj)
{
	assert(obj);
	m_noClipList.AddItemToEnd(obj);
}

void CSceneManager::AddRenderableObjectToScene(IRenderable* obj)
{
	assert(obj);
	m_quadTree.AddObjectToTree(obj);
}

//////////////////////////////////////////////////////////////////////////
// Render Functions
//////////////////////////////////////////////////////////////////////////

void CSceneManager::Render( CRenderEngine &rndr )
{
	assert(m_debugMesh);
	CShaderManager &shaderMgr = rndr.GetShaderManager();
	LPDIRECT3DDEVICE9 device = rndr.GetDevice();

	/* render the quad tree nodes for debug purposes */
	shaderMgr.SetVertexShader(PASS_PRIM_COLORED);
	shaderMgr.SetPixelShader(PASS_PRIM_COLORED);

	shaderMgr.SetViewProjection(PASS_PRIM_COLORED, m_camera.GetViewMatrix(), m_camera.GetProjectionMatrix());
	device->SetFVF(m_debugMesh->GetFVF());
	device->SetTexture(0, 0);

	LPDIRECT3DINDEXBUFFER9 indices;
	LPDIRECT3DVERTEXBUFFER9 vertices;
	m_debugMesh->GetIndexBuffer( &indices );
	m_debugMesh->GetVertexBuffer( &vertices );
	device->SetIndices( indices );
	device->SetStreamSource(0, vertices, 0, m_debugMesh->GetVertexSizeInBytes());

	// stencil setup
	device->SetRenderState( D3DRS_STENCILENABLE, TRUE );
	device->SetRenderState( D3DRS_STENCILFUNC, D3DCMP_EQUAL );
	device->SetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP );
	device->SetRenderState( D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP );
	device->SetRenderState( D3DRS_STENCILREF, 0 );
	device->SetRenderState( D3DRS_STENCILMASK, 0 );

	// background
	shaderMgr.SetDrawColor(PASS_PRIM_COLORED, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f));
	for(int i = 0, j = m_quadTree.GetQuadTreeSize(); i < j; i++)
	{
		shaderMgr.SetWorldTransform(PASS_PRIM_COLORED, m_quadTree.GetWorldTransformByQuadIndex(i) );
		device->DrawPrimitive(D3DPT_LINESTRIP, 0, m_debugMesh->GetNumVertices());		
	}

	// decal
	device->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
	device->SetRenderState( D3DRS_ZENABLE, FALSE );

	// find the leaf quad the camera is currently within, and display it highlighted with red
	int cameraQuadIndex = m_quadTree.FindLeafQuadByPoint(m_camera.Get3DPosition());
	shaderMgr.SetWorldTransform( PASS_PRIM_COLORED, m_quadTree.GetWorldTransformByQuadIndex(cameraQuadIndex) );
	shaderMgr.SetDrawColor(PASS_PRIM_COLORED, D3DXCOLOR(1.0f, 0.2f, 0.2f, 1.0f));
	device->DrawPrimitive(D3DPT_LINESTRIP, 0, m_debugMesh->GetNumVertices());

	// stencil tear down
	device->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
	device->SetRenderState( D3DRS_ZENABLE, TRUE );
	device->SetRenderState( D3DRS_STENCILENABLE, FALSE );

	COM_SAFERELEASE(indices);
	COM_SAFERELEASE(vertices);
}

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