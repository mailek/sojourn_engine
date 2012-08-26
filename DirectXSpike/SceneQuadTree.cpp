/********************************************************************
	created:	2012/04/16
	filename: 	SceneQuadTree.cpp
	author:		Matthew Alford
	
	purpose:	
*********************************************************************/
#include "StdAfx.h"
#include "SceneQuadTree.h"
#include "Camera.h"

static const int FARPLANEDISTMOD			= 500; // draw distance of camera
static const int DEPTHOFSPHERETEST			= 4;
static const bool OptimizeCollisionFor2D	= true; // perform collision only in XZ plane
static const int MAX_OBJECT_NODES			= 4000;

//////////////////////////////////////////////////////////////////////////
// Setup Functions
//////////////////////////////////////////////////////////////////////////

CSceneQuadTree::CSceneQuadTree(void) : m_currentFrame(0)
{
	memset(m_quadTreeArray, 0, sizeof(m_quadTreeArray));
}

CSceneQuadTree::~CSceneQuadTree(void)
{
	for(int i = 0; i < MAXQUADS; i++)
		ptr_safedelete(m_quadTreeArray[i].objects);
}

//////////////////////////////////////////////////////////////////////////
// Update Functions
//////////////////////////////////////////////////////////////////////////

/************************************************
*   Name:   CSceneQuadTree::AddObjectToTree
*   Desc:   Insert a new object sorted into quad
*           tree.
************************************************/
void CSceneQuadTree::AddObjectToTree(IRenderable* obj)
{
	const int MAXLEAVES = 700;
	static NODE_INDEX_TYPE leavesCollidingWithSphere[MAXLEAVES];
	DASSERT(obj);

	// find the leaf quad that contains the object's center
	D3DXVECTOR3 pos(0,0,0);
	D3DXVec3TransformCoord(&pos, &pos, &obj->GetWorldTransform());
	int index = FindLeafQuadByPoint(pos);

	obj->SetLastRenderFrame(m_currentFrame);

	if(m_quadTreeArray[index].objects == NULL)
		m_quadTreeArray[index].objects = new std::vector<IRenderable*>();
	
	m_quadTreeArray[index].objects->push_back(obj);	

	// find the additional quad leaves that intersect the object's sphere boundary
	int leafCount = FindLeafListQuadsByBoundSphere(obj->GetBoundingSphere(), leavesCollidingWithSphere);
	int nodeIndex = 0;
	DASSERT(leafCount < MAXLEAVES);
	
	for(int i = 0; i < leafCount; i++)
	{
		nodeIndex = leavesCollidingWithSphere[i];
		if(nodeIndex == index) // already inserted into the position-collide leaf, later this needs to add a flag on the leaf object to identify reference vs. actual (center point)
			continue;

		if(m_quadTreeArray[nodeIndex].objects == NULL)
			m_quadTreeArray[nodeIndex].objects = new std::vector<IRenderable*>();

		m_quadTreeArray[nodeIndex].objects->push_back(obj); // create a new reference
	}

}

/************************************************
*   Name:   CSceneQuadTree::AddObjectsToRenderListsByQuadIndex
*   Desc:   Grabs all the objects in a given quad 
*           node index, sorts into opaque and 
*           transparent lists, and optionally get 
*           all objects from children without 
*           scrutiny.
************************************************/
void CSceneQuadTree::AddObjectsToRenderListsByQuadIndex(long *index, long numNodes, const D3DXMATRIX &viewMatrix, SceneMgrRenderList &opaque, SceneMgrSortList &transparent, bool recurseGrabChildren/*=false*/)
{
	long idx			= *index;
	IRenderable *obj	= NULL;

	for( long i = 0; i < numNodes; i++, idx = *++index )
	{
		if(m_quadTreeArray[idx].objects == NULL)
			continue;
		
		for(std::vector<IRenderable*>::iterator it = m_quadTreeArray[idx].objects->begin(), _it = m_quadTreeArray[idx].objects->end(); it != _it; ++it)
		{
			obj = *it;

			/* if this object has already been rendered this frame then skip adding to render list 
			   (saves duplicate render of objects that are in multiple quad leaves) */
			if(obj->GetLastRenderFrame() == m_currentFrame) 
				continue;

			obj->SetLastRenderFrame(m_currentFrame);

			if(obj->IsTransparent())
			{
				ZSortableRenderable zsr;
				::ZeroMemory(&zsr, sizeof(zsr));
				zsr.p = obj;
				D3DXVECTOR3 v(0,0,0);
				D3DXVec3TransformCoord(&v, &v, &D3DXMATRIX(obj->GetWorldTransform() * viewMatrix));
				zsr.viewSpaceZ = v.z;
				
				transparent.push_back(zsr);
			}
			else
				opaque.push_back(obj);
		}
	}

}

//////////////////////////////////////////////////////////////////////////
// Query Functions
//////////////////////////////////////////////////////////////////////////

/************************************************
*   Name:   CSceneQuadTree::GetRenderListsByCamera
*   Desc:   Find all the objects inside the camera's
*           frustum.
************************************************/
void CSceneQuadTree::GetRenderListsByCamera( const CCamera& camera, SceneMgrRenderList &opaque, SceneMgrSortList &transparent )
{
	D3DXMATRIX view = camera.GetViewMatrix();
	Frustum_Camera frustum = camera.GetFrustum();

	NODE_INDEX_TYPE objectNodes[MAX_OBJECT_NODES];
	long nodeCnt = FrustumRecurseCollideQuadChildren( 0, frustum, objectNodes );
	DASSERT(nodeCnt<MAX_OBJECT_NODES);

	AddObjectsToRenderListsByQuadIndex( objectNodes, nodeCnt, view, opaque, transparent );
}

/************************************************
*   Name:   CSceneQuadTree::GetAllRenderObjects
*   Desc:   Retrieve all the objects in the quad
*           tree structure.
************************************************/
void CSceneQuadTree::GetAllRenderObjects( const D3DXMATRIX &view, SceneMgrRenderList &opaque, SceneMgrSortList &transparent )
{
	NODE_INDEX_TYPE objectNodes[MAX_OBJECT_NODES];
	long nodeCnt = GetAllOccupiedDescendents( 0, objectNodes );
	DASSERT(nodeCnt<MAX_OBJECT_NODES);

	AddObjectsToRenderListsByQuadIndex( objectNodes, nodeCnt, view, opaque, transparent );
}