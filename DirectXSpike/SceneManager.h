#pragma once
//////////////////////////////////////////////////////////////////////////
// SceneManager.h - 2011 Matthew Alford
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////
// Includes
//////////////////////////////////////
#include "SceneQuadTree.h"
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
	BRUTEFORCE, 
	FRUSTUMCLIP, 
	CAMERAQUAD
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
	SceneMgrRenderList			m_opaqueList;
	SceneMgrSortList			m_transparentList;
	std::vector<IRenderable*>	m_noClipList;
	BaseModel*					m_debugMesh;
	CSceneQuadTree				m_quadTree;
	bool						m_bDrawDebugQuadTree;              // flag to render or hide quad tree lines
	D3DXMATRIX					m_lastViewSpace;
	CCamera						m_camera;
	int							m_clipStrategy;		// the current clip strategy for building a render list of objects

	void UpdateDrawLists();

public:
	void GetOpaqueDrawListF2B(SceneMgrRenderList &list);
	void GetTransparentDrawListB2F(SceneMgrSortList &list);
	void AddRenderableObjectToScene(IRenderable* obj);
	void AddNonclippableObjectToScene(IRenderable* obj);
	void Setup(LPDIRECT3DDEVICE9 device, CTerrain& terrain, CMeshManager& meshMgr);
	
	inline void DrawDebugQuadTree(bool draw) {m_bDrawDebugQuadTree = draw;}
	inline void SetNextClipStrategy(int strategy = -1) { if(strategy >= 0) m_clipStrategy = strategy; else { m_clipStrategy++; m_clipStrategy = m_clipStrategy > CAMERAQUAD ? BRUTEFORCE : m_clipStrategy; } }
	inline CCamera& GetDefaultCamera() {return m_camera;}
	
	virtual D3DXMATRIX GetWorldTransform() {D3DXMATRIX worldMatrix;	D3DXMatrixIdentity(&worldMatrix); return worldMatrix;}
	virtual void Render( CRenderEngine &rndr );
	virtual bool IsTransparent() {return false;}
	virtual void SetLastRenderFrame(UINT frameNum) {};
	virtual UINT GetLastRenderFrame() {return 0;}
	virtual Sphere_PosRad GetBoundingSphere() { Sphere_PosRad s; ::ZeroMemory(&s, sizeof(s)); return s; }
};