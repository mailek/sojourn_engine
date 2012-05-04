#pragma once
/********************************************************************
	created:	2012/04/16
	filename: 	Camera.h
	author:		Matthew Alford
	
	purpose:	
	notes:		Handles gameevents.
*********************************************************************/
//////////////////////////////////////
// Includes
//////////////////////////////////////
#include "SceneQuadTree.h"
#include "Camera.h"
#include "gameevents.h"

//////////////////////////////////////
// Forward Declarations
//////////////////////////////////////
class CPlayer;
class CTerrainField;

using namespace GameEvents;

//////////////////////////////////////
// Class Definition
//////////////////////////////////////
class CCamera
{
public:
	CCamera(void);
	~CCamera(void) {};

private:
	D3DXVECTOR3			m_vecPosLocalOffset;
	D3DXVECTOR3			m_vecTargetLookAt;
	D3DXVECTOR3			m_vecLookAtLocalOffset;

	Frustum_Camera		m_frustum;
	
	D3DXMATRIX			m_projectionMatrix;
	D3DXMATRIX			m_viewMatrix;

	IEventHandler	   *m_pAttachParent;  // object which camera is attached to
	CTerrainField	   *m_pTerrain; // camera needs terrain to find ground clamp

public:
	void Update(float elapsedMillis);
	void AdjustFOVAngle( float deltaRads );

	inline void SetCameraAttach(IEventHandler *parent) {m_pAttachParent = parent;}
	inline void SetTerrainToClamp(CTerrainField *terrain) {m_pTerrain = terrain;}

	inline void SetViewPort(unsigned int viewWidth, unsigned int viewHeight) { m_frustum.ASPECT = (float)viewWidth/(float)viewHeight; D3DXMatrixPerspectiveFovLH(&m_projectionMatrix, m_frustum.FOVANGLE, m_frustum.ASPECT, m_frustum.NEARDIST, m_frustum.FARDIST); }
	inline void Set3DPosition(D3DXVECTOR3 &pos) {m_frustum.cameraPos = pos;}
	inline void SetLocalPosition(D3DXVECTOR3 &pos) {m_vecPosLocalOffset = pos;}

	inline D3DXMATRIX GetViewMatrix() const { return m_viewMatrix; }
	inline D3DXMATRIX GetProjectionMatrix() const {return m_projectionMatrix;}
	inline D3DXMATRIX GetViewProjectionMatrix() const {return m_viewMatrix*m_projectionMatrix;}
	inline Frustum_Camera GetFrustum() const { return m_frustum; }
	inline D3DXVECTOR3 Get3DPosition() const { return  m_frustum.cameraPos; } 
};