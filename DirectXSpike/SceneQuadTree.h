#pragma once
/********************************************************************
	created:	2012/04/16
	filename: 	SceneQuadTree.h
	author:		Matthew Alford
	
	purpose:	
*********************************************************************/
//////////////////////////////////////
// Includes
//////////////////////////////////////
#include "quadtreegroundclamped.h"
#include "IRenderable.h"
#include "TerrainMiscTypes.h"

//////////////////////////////////////
// Forward Declarations
//////////////////////////////////////
class CCamera;

//////////////////////////////////////
// Type Definitions
//////////////////////////////////////
typedef class ZSortableRenderable{
public:
	IRenderable*	p;
	float			viewSpaceZ;
	bool operator<(ZSortableRenderable &comp) {return viewSpaceZ < comp.viewSpaceZ;}
} ZSR;

typedef std::list<ZSortableRenderable> SceneMgrSortList;

//////////////////////////////////////
// Class Definition
//////////////////////////////////////
class CSceneQuadTree : public QuadTree_GroundClamped<IRenderable, CTerrainContainer>
{
public:
	CSceneQuadTree(void);
	virtual ~CSceneQuadTree(void);

	void AddObjectsToRenderListsByQuadIndex(long *index, long numNodes, const D3DXMATRIX &viewMatrix, SceneMgrRenderList &opaque, SceneMgrSortList &transparent, bool recurseGrabChildren=false);
	void GetRenderListsByCamera(const CCamera &camera, SceneMgrRenderList &opaque, SceneMgrSortList &transparent);
	void AddObjectToTree(IRenderable* obj);
	void GetAllRenderObjects( const D3DXMATRIX &view, SceneMgrRenderList &opaque, SceneMgrSortList &transparent );

	inline void NextFrame() {m_currentFrame++;}

private:
	UINT						m_currentFrame;

};
