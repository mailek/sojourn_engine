#pragma once

#include "gameevents.h"

class CGhostCamera : public IEventHandler
{
public:
	CGhostCamera(void);
	~CGhostCamera(void);

private:
	D3DXVECTOR3				m_vecEuler; // in radians
	D3DXVECTOR3				m_vecPos;
	D3DXVECTOR3				m_vecVelocity;
	D3DXVECTOR3				m_vecRotationVelocity;
	DWORD					m_movementState;

public:
	inline D3DXVECTOR3 GetPosition3D() {return m_vecPos;}

	// IEventHandler
	virtual bool HandleEvent( UINT eventId, void* data, UINT data_sz );

private:
	void Update(float deltaTime);
	inline D3DXVECTOR3 GetRotationRadians() {return m_vecEuler;}
	inline void SetXRotationRadians(float xRot) {m_vecEuler.x = xRot;}
	inline void SetYRotationRadians(float yRot) {m_vecEuler.y = yRot;}
	inline void SetZRotationRadians(float zRot) {m_vecEuler.z = zRot;}
	
};
