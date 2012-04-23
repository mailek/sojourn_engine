/********************************************************************
	created:	2012/04/16
	filename: 	SceneQuadTree.cpp
	author:		Matthew Alford
	
	purpose:	
*********************************************************************/
#include "StdAfx.h"
#include "SceneQuadTree.h"
#include "Camera.h"
#include "MathUtil.h"

static const int FARPLANEDISTMOD				= 500; // draw distance of camera
static const int DEPTHOFSPHERETEST				= 4;
static const bool OptimizeCollisionFor2D		= true; // perform collision only in XZ plane
static const int MAX_OBJECT_NODES				= 4000;

//////////////////////////////////////////////////////////////////////////
// Setup Functions
//////////////////////////////////////////////////////////////////////////

CSceneQuadTree::CSceneQuadTree(void) : m_currentFrame(0)
{
}

CSceneQuadTree::~CSceneQuadTree(void)
{
}

//////////////////////////////////////////////////////////////////////////
// Update Functions
//////////////////////////////////////////////////////////////////////////

void CSceneQuadTree::AddObjectToTree(IRenderable* obj)
{
	const int MAXLEAVES(200);
	static NODE_INDEX_TYPE leavesCollidingWithSphere[MAXLEAVES];
	assert(obj);

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
	assert(leafCount < MAXLEAVES);
	
	for( int i = 0; i < leafCount; i++ )
	{
		int nodeIndex = leavesCollidingWithSphere[i];
		if(nodeIndex == index) // already inserted into the position-collide leaf, later this needs to add a flag on the leaf object to identify reference vs. actual (center point)
			continue;

		if(m_quadTreeArray[nodeIndex].objects == NULL)
			m_quadTreeArray[nodeIndex].objects = new std::vector<IRenderable*>();

		m_quadTreeArray[nodeIndex].objects->push_back(obj); // create a new reference
	}

}

// Grabs all the objects in a given quad node index, sorts into opaque and transparent lists, and optionally get all objects from children without scrutiny
void CSceneQuadTree::AddObjectsToRenderListsByQuadIndex(long *index, long numNodes, const D3DXMATRIX &viewMatrix, SceneMgrRenderList &opaque, SceneMgrSortList &transparent, bool recurseGrabChildren/*=false*/)
{
	long idx = *index;
	for( long i = 0; i < numNodes; i++, idx = *++index )
	{
		if(m_quadTreeArray[idx].objects == NULL)
			continue;
		
		for(std::vector<IRenderable*>::iterator it = m_quadTreeArray[idx].objects->begin(), _it = m_quadTreeArray[idx].objects->end(); it != _it; it++)
		{
			// if we already rendered this object this frame then skip adding to render list (saves duplicate render of objects that are in multiple quad leaves)
			if((*it)->GetLastRenderFrame() == m_currentFrame) 
				continue;

			(*it)->SetLastRenderFrame(m_currentFrame);

			if((*it)->IsTransparent())
			{
				ZSortableRenderable zsr;
				::ZeroMemory(&zsr, sizeof(zsr));
				zsr.p = (*it);
				D3DXVECTOR3 v(0,0,0);
				D3DXVec3TransformCoord(&v, &v, &D3DXMATRIX((*it)->GetWorldTransform() * viewMatrix));
				zsr.viewSpaceZ = v.z;
				
				transparent.push_back(zsr);
			}
			else
				opaque.push_back(*it);
		}
	}

}

//////////////////////////////////////////////////////////////////////////
// Query Functions
//////////////////////////////////////////////////////////////////////////
void CSceneQuadTree::GetRenderListsByCamera( const CCamera& camera, SceneMgrRenderList &opaque, SceneMgrSortList &transparent )
{
	D3DXMATRIX view = camera.GetViewMatrix();
	Frustum_Camera frustum = camera.GetFrustum();

	NODE_INDEX_TYPE objectNodes[MAX_OBJECT_NODES];
	long nodeCnt = FrustumRecurseCollideQuadChildren( 0, frustum, objectNodes );
	assert(nodeCnt<MAX_OBJECT_NODES);

	AddObjectsToRenderListsByQuadIndex( objectNodes, nodeCnt, view, opaque, transparent );
}

void CSceneQuadTree::GetAllRenderObjects( const D3DXMATRIX &view, SceneMgrRenderList &opaque, SceneMgrSortList &transparent )
{
	NODE_INDEX_TYPE objectNodes[MAX_OBJECT_NODES];
	long nodeCnt = GetAllOccupiedDescendents( 0, objectNodes );
	assert(nodeCnt<MAX_OBJECT_NODES);

	AddObjectsToRenderListsByQuadIndex( objectNodes, nodeCnt, view, opaque, transparent );
}



