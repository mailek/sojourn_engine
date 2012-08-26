#pragma once
/********************************************************************
	created:	2012/04/23
	filename: 	LevelManager.h
	author:		Matthew Alford
	
	purpose:	Maintains the state of the level including static 
				level objects, dynamic actor-driven objects, terrain,
				cameras, environment, and other world entities.  Registers
				objects for collision with the collision system, and
				for rendering with the scene graph.
*********************************************************************/

//////////////////////////////////////
// Include
//////////////////////////////////////
#include "IRenderable.h"
#include "doublelinkedlist.h"
#include "ICollidable.h"
#include "TerrainField.h"
#include "Skybox.h"

//////////////////////////////////////
// Forward Declarations
//////////////////////////////////////
class BaseModel;
class CMeshManager;
class CSceneManager;
class CRenderEngine;
class CCollisionManager;
class CSceneManager;

using namespace GameEvents;

//////////////////////////////////////
// Class Definition
//////////////////////////////////////
class CLevelManager : public IEventHandler
{
	class LevelObject;
	static const int MAX_LIGHTS = 20;

	//////////////////////////////////
	// LightObject
	//////////////////////////////////
	typedef struct _LightObject 
	{
		bool				used;
		bool				isOn;
		D3DLIGHT9			light;
	} LightObject;

	//////////////////////////////////
	// Level State
	//////////////////////////////////
	typedef struct _LevelState
	{
		CDoubleLinkedList<LevelObject>	staticLevelObjects;		/* static object collection				*/
		CSkybox					        skyDome;				/* follow skydome						*/
		LightObject						lights[MAX_LIGHTS];		/* active lights in scene				*/
		CTerrainField					terrain;				/* terrain grid							*/
	} LevelState;

public:
	CLevelManager(void);
	~CLevelManager(void);

	void LoadDefaultLevel(CRenderEngine *renderEngine);
	void RegisterStaticCollision(CCollisionManager* cm);
	CTerrainField* GetTerrain() {return &m_levelState.terrain;}

	/* IEventHandler */
	virtual bool HandleEvent( UINT eventId, void* data, UINT data_sz );

private:
	LevelState						m_levelState;			/* current level state					*/
	CSceneManager				   *m_pSceneMgr;			/* reference to scene mgr to tend when 
															   new renderable objects come into 
															   level or when terrain changes		*/
	void AddDirLight(Vector_3 lightDir, Color_4 lightColor);
	void Update( float elapsedMillis );
	void RefreshSceneManager();
	void OnTerrainGridChanged( CTerrainContainer* terrainGrid );

	//////////////////////////////////
	// LevelObject
	//////////////////////////////////
	typedef class LevelObject : public IRenderable, public ICollidable 
	{
		friend class CLevelManager;
	public:
		LevelObject(void);
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

		/* ICollidable */
		virtual void GetCollideSphere( Sphere_PosRad& out ) {out = GetBoundingSphere();}
		virtual void HandleCollision( ICollidable* other ) {};
	};

};