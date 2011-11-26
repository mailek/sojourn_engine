#pragma once
//////////////////////////////////////////////////////////////////////////
// Player.h - 2011 Matthew Alford
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////
// Includes
//////////////////////////////////////
#include "BaseModel.h"
#include "IRenderable.h"
#include "ICollidable.h"

//////////////////////////////////////
// Forward Declarations
//////////////////////////////////////
class CShaderManager;
class CMeshManager;
class CTerrain;

//////////////////////////////////////
// Type Definitions
//////////////////////////////////////
typedef enum {
	MOVE_FORWARD =  (1<<0), 
	MOVE_BACKWARD = (1<<1), 
	MOVE_LEFT =		(1<<2), 
	MOVE_RIGHT =	(1<<3)
	};

typedef struct {
	float collideSphereRadius;	// size of collide sphere
	float maxRotateSpeed;		// radians per sec
	} PlayerProperties;

//////////////////////////////////////
// Class Definition
//////////////////////////////////////
class CPlayer : public IRenderable, public ICollidable
{
public:
	CPlayer(void);
	~CPlayer(void);

private:
	D3DXVECTOR3				m_vecScale;
	D3DXVECTOR3				m_vecPos;
	D3DXVECTOR3				m_vecEuler; // in radians
	D3DXVECTOR3				m_vecVelocity;
	DWORD					m_movementState;
	D3DXVECTOR3				m_vecLastPos;
	D3DXVECTOR3				m_vecRotationVelocity;

	BaseModel*				m_pModel;
	CTerrain*				m_pTerrain;
	PlayerProperties		m_properties;

public:
	void Update(float deltaTime);
	void Setup(CMeshManager& meshMgr);
	void GroundClampTerrain();	
	void SetMovementState(int state);
	void ClearMovementState(int state);
	void SetPosition3D(D3DXVECTOR3 &pos);
	
	inline D3DXVECTOR3 GetPosition3D() {return m_vecPos;}
	inline D3DXVECTOR3 GetRotationRadians() {return m_vecEuler;}
	inline void SetXRotationRadians(float xRot) {m_vecEuler.x = xRot;}
	inline void SetYRotationRadians(float yRot) {m_vecEuler.y = yRot;}
	inline void SetZRotationRadians(float zRot) {m_vecEuler.z = zRot;}
	inline void SetScale(D3DXVECTOR3 &scale) {m_vecScale = scale;}
	inline void SetGroundClampTerrain(CTerrain &terrain) {m_pTerrain = &terrain; GroundClampTerrain();}

	// IRenderable
	virtual void Render( CRenderEngine &rndr );
	virtual D3DXMATRIX GetWorldTransform();
	virtual bool IsTransparent() { if(m_pModel==NULL) return false; return m_pModel->IsTransparent();}
	virtual void SetLastRenderFrame(UINT frameNum) {};
	virtual UINT GetLastRenderFrame() {return 0;}
	virtual Sphere_PosRad GetBoundingSphere() { Sphere_PosRad s = m_pModel->GetSphereBounds(); s.pos += m_vecPos; return s; }

	// ICollidable
	virtual void GetCollideSphere( Sphere_PosRad& out ) { ::ZeroMemory(&out, sizeof(out)); out.pos = m_vecPos; out.radius = m_properties.collideSphereRadius;}

};