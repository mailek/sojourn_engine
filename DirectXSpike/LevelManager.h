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
#include "mathutil.h"

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
		friend class CLevelManager;
	public:
		SceneObject(void);
		virtual void Render( CRenderEngine &rndr );
		virtual Sphere_PosRad GetBoundingSphere();
		virtual bool IsTransparent();

		virtual D3DXMATRIX GetWorldTransform() {return m_transform.GetTransform();}
		virtual void SetLastRenderFrame(UINT frameNum) {m_lastFrame = frameNum;}
		virtual UINT GetLastRenderFrame() {return m_lastFrame;}
		virtual void SetLocalPosition(D3DXVECTOR3 &translation) {m_transform.SetTranslation(translation);}
		virtual void SetScale(D3DXVECTOR3 &scale) {m_transform.SetScale(scale);}
		virtual void SetXRotationRadians(float xRot) {m_transform.SetXRotationRadians(xRot);}
		virtual void SetYRotationRadians(float yRot) {m_transform.SetYRotationRadians(yRot);}
		virtual void SetZRotationRadians(float zRot) {m_transform.SetZRotationRadians(zRot);}

	private:
		ComplexTransform	m_transform;
		BaseModel          *m_pMesh;
		bool				m_bTransparent;
		UINT				m_lastFrame;

		// ICollidable
		virtual void GetCollideSphere( Sphere_PosRad& out ) {out = GetBoundingSphere();}
		virtual void HandleCollision( ICollidable* other ) {};
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