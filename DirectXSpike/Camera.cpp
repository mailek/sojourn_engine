/********************************************************************
	created:	2012/04/16
	filename: 	Camera.cpp
	author:		Matthew Alford
	
	purpose:	
*********************************************************************/
#include "StdAfx.h"
#include "Camera.h"
#include "Player.h"
#include "Terrain.h"

const float FARPLANEDISTANCE	= DRAW_DIST;
const float NEARPLANEDISTANCE	= 1.0f;
const float FOV_DEFAULT_ANGLE	= 1.3f;
const float FOV_MIN_ANGLE_RAD	= 0.3f;
const float FOV_MAX_ANGLE_RAD	= HALF_PI*1.3f;

//////////////////////////////////////////////////////////////////////////
// Setup Functions
//////////////////////////////////////////////////////////////////////////

CCamera::CCamera(void) : m_pAttachParent(NULL),
						m_pTerrain(NULL)
{
	::ZeroMemory(&m_frustum, sizeof(m_frustum));
	::ZeroMemory(m_vecTargetLookAt, sizeof(m_vecTargetLookAt));

	/* Initialize the frustum */
	m_frustum.FARDIST = FARPLANEDISTANCE;
	m_frustum.NEARDIST = NEARPLANEDISTANCE;
	m_frustum.FOVANGLE = FOV_DEFAULT_ANGLE;
	m_frustum.cameraX = D3DXVECTOR3(1,0,0);
	m_frustum.cameraY = D3DXVECTOR3(0,1,0);
	m_frustum.cameraZ = D3DXVECTOR3(0,0,1);

	/*m_vecPosLocalOffset = D3DXVECTOR3(-5.0f, 12.0f, -20.0f)*/;
	m_vecLookAtLocalOffset = D3DXVECTOR3(0.0f, 0.0f, 25.0f);

	D3DXMatrixIdentity(&m_projectionMatrix);
	D3DXMatrixIdentity(&m_viewMatrix);
}

//////////////////////////////////////////////////////////////////////////
// Update Functions
//////////////////////////////////////////////////////////////////////////

void CCamera::Update(float elapsedMillis)
 {
	D3DXVECTOR3 augPos =  D3DXVECTOR3(0,0,m_vecPosLocalOffset.z);
	
	if(m_pAttachParent)
	{
		Vector_3 parentRot, parentPos;
		m_pAttachParent->HandleEvent(EVT_GETFACINGVEC, &parentRot, sizeof(parentRot));
		m_pAttachParent->HandleEvent(EVT_GETPOSITIONVEC, &parentPos, sizeof(parentPos));

		D3DXMATRIX yRotate, xRotate;
		D3DXMatrixRotationX(&xRotate, parentRot.x);
		D3DXMatrixRotationY(&yRotate, parentRot.y);

		D3DXVec3TransformCoord(&augPos, &augPos, &xRotate);
		augPos += D3DXVECTOR3(m_vecPosLocalOffset.x, 0, 0);
		D3DXVec3TransformCoord(&augPos, &augPos, &yRotate);

		augPos += parentPos;
		augPos += D3DXVECTOR3(0,m_vecPosLocalOffset.y, 0);
		m_frustum.cameraPos = augPos;

		//m_vecTargetLookAt = parentPos;
		m_vecTargetLookAt = D3DXVECTOR3(0,0,m_vecLookAtLocalOffset.z);
		D3DXVec3TransformCoord(&m_vecTargetLookAt, &m_vecTargetLookAt, &xRotate);
		m_vecTargetLookAt += D3DXVECTOR3(m_vecPosLocalOffset.x, 0, 0);
		D3DXVec3TransformCoord(&m_vecTargetLookAt, &m_vecTargetLookAt, &yRotate);
		m_vecTargetLookAt += parentPos;
		m_vecTargetLookAt += D3DXVECTOR3(0,m_vecPosLocalOffset.y, 0);
	}

	D3DXMatrixLookAtLH(&m_viewMatrix, &m_frustum.cameraPos, &m_vecTargetLookAt, &D3DXVECTOR3(0,1,0));
	//Matrix4x4_LookAtQuaternion(&m_viewMatrix, &m_frustum.cameraPos, &m_vecTargetLookAt);
	m_frustum.cameraX = D3DXVECTOR3(m_viewMatrix._11, m_viewMatrix._21, m_viewMatrix._31); // right axis
	m_frustum.cameraY = D3DXVECTOR3(m_viewMatrix._12, m_viewMatrix._22, m_viewMatrix._32); // up axis
	m_frustum.cameraZ = D3DXVECTOR3(m_viewMatrix._13, m_viewMatrix._23, m_viewMatrix._33); // look axis

	// renormalize the unit vectors
	//D3DXVec3Normalize(&m_vecZ, &m_vecZ);
	//D3DXVec3Cross(&m_vecWorldUp, &m_vecLook, &m_vecRight);
	//D3DXVec3Normalize(&m_vecY, &m_vecY);
	//D3DXVec3Cross(&m_vecRight, &m_vecWorldUp, &m_vecLook);
	//D3DXVec3Normalize(&m_vecX, &m_vecX);
}

void CCamera::AdjustFOVAngle( float deltaRads )
{ 
	m_frustum.FOVANGLE += deltaRads;
	m_frustum.FOVANGLE = fclamp( m_frustum.FOVANGLE, FOV_MIN_ANGLE_RAD, FOV_MAX_ANGLE_RAD );
	D3DXMatrixPerspectiveFovLH(
		&m_projectionMatrix, 
		m_frustum.FOVANGLE, 
		m_frustum.ASPECT, 
		m_frustum.NEARDIST, 
		m_frustum.FARDIST
		); 
}