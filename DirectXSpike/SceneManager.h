#pragma once
//////////////////////////////////////////////////////////////////////////
// SceneManager.h - 2011 Matthew Alford
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////
// Includes
//////////////////////////////////////
#include "SceneQuadTree.h"
#include "doublelinkedlist.h"
#include "Camera.h"

//////////////////////////////////////
// Forward Declarations
//////////////////////////////////////
class CCamera;
class CTerrain;
class CMeshManager;
class BaseModel;

//////////////////////////////////////
// Constants
//////////////////////////////////////

enum {
	FIRST_STRATEGY,
	BRUTEFORCE = FIRST_STRATEGY, 
	FRUSTUMCLIP, 
	CAMERAQUAD,
	STRATEGY_CNT
};

enum { 
	RENDER_TERRAIN, 
	RENDER_SKY 
};

//////////////////////////////////////
// Class Definition
//////////////////////////////////////
class CSceneManager : public IRenderable
{
public:
	CSceneManager(void);
	~CSceneManager(void);

private:
	SceneMgrRenderList				m_opaqueList;
	SceneMgrSortList				m_transparentList;
	CDoubleLinkedList<IRenderable>	m_noClipList;
	BaseModel*						m_debugMesh;
	CSceneQuadTree					m_quadTree;
	Matrix4x4						m_lastViewSpace;
	CCamera							m_camera;
	int								m_clipStrategy;			// the current clip strategy for building a render list of objects

	void UpdateDrawLists();

public:
	void GetOpaqueDrawListF2B(SceneMgrRenderList &list);
	void GetTransparentDrawListB2F(SceneMgrSortList &list);
	void AddRenderableObjectToScene(IRenderable* obj);
	void AddNonclippableObjectToScene(IRenderable* obj);
	void Setup(LPDIRECT3DDEVICE9 device, CTerrain& terrain, CMeshManager& meshMgr);
	
    void SetNextClipStrategy(int strategy = -1);
	inline CCamera& GetDefaultCamera() {return m_camera;}
	
	virtual Matrix4x4 GetWorldTransform() {Matrix4x4 worldMatrix;	Matrix4x4_LoadIdentity(&worldMatrix); return worldMatrix;}
	virtual void Render( CRenderEngine &rndr );
	virtual bool IsTransparent() {return false;}
	virtual void SetLastRenderFrame(UINT frameNum) {};
	virtual UINT GetLastRenderFrame() {return 0;}
	virtual Sphere_PosRad GetBoundingSphere() { Sphere_PosRad s; ::ZeroMemory(&s, sizeof(s)); return s; }
};