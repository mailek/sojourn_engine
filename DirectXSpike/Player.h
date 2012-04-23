#pragma once
/********************************************************************
	created:	2012/04/23
	filename: 	Player.h
	author:		Matthew Alford
	
	purpose:	
*********************************************************************/

//////////////////////////////////////
// Includes
//////////////////////////////////////
#include "BaseModel.h"
#include "IRenderable.h"
#include "ICollidable.h"
#include "GameEvents.h"
#include "mathutil.h"

//////////////////////////////////////
// Forward Declarations
//////////////////////////////////////
class CShaderManager;
class CMeshManager;
class CTerrain;

//////////////////////////////////////
// Type Definitions
//////////////////////////////////////

typedef struct _PlayerProperties
{
	float collideSphereRadius;	// size of collide sphere
	float maxRotateSpeed;		// radians per sec
} PlayerProperties;

//////////////////////////////////////
// Class Definition
//////////////////////////////////////
class CPlayer : public IRenderable, public ICollidable, public IEventHandler
{
public:
	CPlayer(void);
	~CPlayer(void);

private:
	Vector_3				m_vecVelocity;
	DWORD					m_movementState;
	Vector_3				m_vecLastPos;
	Vector_3				m_vecRotationVelocity;

	BaseModel*				m_pModel;
	CTerrain*				m_pTerrain;
	PlayerProperties		m_properties;
	ComplexTransform		m_transform;

public:
	void Setup(CMeshManager& meshMgr);
	void GroundClampTerrain();	
	void SetPosition3D(Vector_3 &pos);
	
	inline Vector_3 GetPosition3D() {return m_transform.GetTranslation();}
	inline void SetScale(Vector_3 &scale) {m_transform.SetScale(scale);}
	inline void SetGroundClampTerrain(CTerrain &terrain) {m_pTerrain = &terrain; GroundClampTerrain();}

	// IRenderable
	virtual void Render( CRenderEngine &rndr );
	virtual D3DXMATRIX GetWorldTransform();
	virtual bool IsTransparent() { return (m_pModel==NULL && m_pModel->IsTransparent()); }
	virtual void SetLastRenderFrame(UINT frameNum) {};
	virtual UINT GetLastRenderFrame() {return 0;}
	virtual Sphere_PosRad GetBoundingSphere() { Sphere_PosRad s = m_pModel->GetSphereBounds(); s.pos += m_transform.GetTranslation(); return s; }

	// ICollidable
	virtual void GetCollideSphere( Sphere_PosRad& out ) { ::ZeroMemory(&out, sizeof(out)); out.pos = m_transform.GetTranslation(); out.radius = m_properties.collideSphereRadius;}
	virtual void HandleCollision( ICollidable* other );

	// IEventHandler
	virtual bool HandleEvent( UINT eventId, void* data, UINT data_sz );

private:
	void Update(float deltaTime);
	inline Vector_3 GetRotationRadians() {return m_transform.GetRotationEulers();}
	inline void SetXRotationRadians(float xRot) {m_transform.SetXRotationRadians(xRot);}
	inline void SetYRotationRadians(float yRot) {m_transform.SetYRotationRadians(yRot);}
	inline void SetZRotationRadians(float zRot) {m_transform.SetZRotationRadians(zRot);}

};