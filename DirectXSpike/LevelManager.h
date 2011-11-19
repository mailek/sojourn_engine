#pragma once
//////////////////////////////////////////////////////////////////////////
// LevelManager.h - 2011 Matthew Alford
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////
// Include
//////////////////////////////////////
#include "IRenderable.h"
#include "doublelinkedlist.h"
#include "ICollidable.h"

//////////////////////////////////////
// Forward Declarations
//////////////////////////////////////
class BaseModel;
class CMeshManager;
class CSceneManager;
class CTerrain;
class CSkybox;
class CRenderEngine;
class CCollisionManager;

//////////////////////////////////////
// Class Definition
//////////////////////////////////////
class CLevelManager
{
	class SceneObject : public IRenderable, public ICollidable {
	public:
		SceneObject(void);
		virtual void Render( CRenderEngine &rndr );
		virtual D3DXMATRIX GetWorldTransform() {return m_transform;}
		virtual bool IsTransparent() {return m_bTransparent;}
		virtual void SetLastRenderFrame(UINT frameNum) {m_lastFrame = frameNum;}
		virtual UINT GetLastRenderFrame() {return m_lastFrame;}
		virtual Sphere_PosRad GetBoundingSphere() {return m_boundSphere;}

		D3DXMATRIX			m_transform;
		BaseModel			*m_pMesh;
		bool				m_bTransparent;
		UINT				m_lastFrame;
		Sphere_PosRad		m_boundSphere;

		// ICollidable
		virtual void GetCollideSphere( Sphere_PosRad& out ) {out = m_boundSphere;}
	};

private:
	CDoubleLinkedList<SceneObject>	m_staticLevelObjects;
	CTerrain					   *m_pTerrain;
	CSkybox					       *m_pSkyDome;

public:
	CLevelManager(void);
	~CLevelManager(void);

	bool LoadDefaultLevel(CRenderEngine *renderEngine, CTerrain **retTerrain, CSkybox **retSkybox);
	void RegisterStaticCollision(CCollisionManager* cm);
};