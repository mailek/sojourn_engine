#include "StdAfx.h"
#include "GhostCamera.h"
#include "camera.h"

//////////////////////////////////////
// Type Definitions
//////////////////////////////////////

typedef enum {
	MOVE_FORWARD_FLAG	= (1<<0), 
	MOVE_BACKWARD_FLAG	= (1<<1), 
	MOVE_LEFT_FLAG		= (1<<2), 
	MOVE_RIGHT_FLAG		= (1<<3),
	/* all flags */
	MOVE_FLAG_MASK		= MOVE_FORWARD_FLAG | MOVE_BACKWARD_FLAG | MOVE_LEFT_FLAG | MOVE_RIGHT_FLAG
	} MovementFlagType;

//////////////////////////////////////////////////////////////////////////
// Setup Functions
//////////////////////////////////////////////////////////////////////////

CGhostCamera::CGhostCamera(void) : m_movementState(0)
{
	::ZeroMemory(m_vecPos, sizeof(m_vecPos));
	::ZeroMemory(m_vecEuler, sizeof(m_vecEuler));
}

CGhostCamera::~CGhostCamera(void)
{
}

//////////////////////////////////////////////////////////////////////////
// Update Functions
//////////////////////////////////////////////////////////////////////////

bool CGhostCamera::HandleEvent(UINT eventId, void* data, UINT data_sz)
{
	static const MovementFlagType movementMap[] = {
		MOVE_FORWARD_FLAG	/* = MOVE_FORWARD*/,
		MOVE_BACKWARD_FLAG	/* = MOVE_BACKWARD*/,
		MOVE_LEFT_FLAG		/* = MOVE_LEFT*/,
		MOVE_RIGHT_FLAG		/* = MOVE_RIGHT*/,
		};

	switch( eventId )
	{
	case EVT_UPDATE:
		assert( data_sz == sizeof(float) );
		assert( data != NULL );
		Update( *(float*)data );
		break;
	case EVT_GETFACINGVEC:
		{
		assert( data_sz == sizeof(GetFacingEventArgType) );
		assert( data != NULL );
		*(GetFacingEventArgType*)data = GetRotationRadians();
		break;
		}
	case EVT_SETFACINGVEC:
		{
		assert( data_sz == sizeof(SetFacingEventArgType) );
		assert( data != NULL );
		SetFacingEventArgType* arg = (SetFacingEventArgType*)data;
		SetXRotationRadians(arg->x);
		SetYRotationRadians(arg->y);
		SetZRotationRadians(arg->z);
		break;
		}
	case EVT_MOVECMD:
		{
		assert( data_sz == sizeof(MovementCommandType) );
		assert( data != NULL );
		MovementCommandType* args = (MovementCommandType*)data;
		if(args->clearFlag)
			m_movementState &= ~movementMap[args->state];
		else
			m_movementState |= movementMap[args->state];
		break;
		}
	case EVT_GETPOSITIONVEC:
		assert( data_sz == sizeof(GetPosEventArgType) );
		assert( data != NULL );
		*(GetPosEventArgType*)data = GetPosition3D();
		break;
	case EVT_CONTROL_LOST_FOCUS:
	case EVT_CONTROL_GAIN_FOCUS:
		m_movementState &= ~MOVE_FLAG_MASK;
		break;
	case EVT_ATTACH_CAMERA:
		assert( data_sz == sizeof(CCamera*) );
		assert( data != NULL );
		((CCamera*)data)->SetLocalPosition(Vector_3(0.0f, 0.0f, -0.25f));
		break;
	default:
		assert(false);
		break;
	}

	return true;
}

void CGhostCamera::Update(float deltaTime)
{
	::ZeroMemory(m_vecVelocity, sizeof(m_vecVelocity));
	if(m_movementState & MOVE_FORWARD_FLAG)
		m_vecVelocity.z += 50.0f;

	if(m_movementState & MOVE_BACKWARD_FLAG)
		m_vecVelocity.z -= 50.0f;

	if(m_movementState & MOVE_RIGHT_FLAG)
		m_vecEuler.y += deltaTime * 2.0f;

	if(m_movementState & MOVE_LEFT_FLAG)
		m_vecEuler.y -= deltaTime * 2.0f;

	D3DXVECTOR3 displacement = m_vecVelocity * deltaTime;

	D3DXMATRIX yRot, xRot;
	D3DXMatrixRotationY(&yRot, m_vecEuler.y);
	D3DXMatrixRotationX(&xRot, m_vecEuler.x);

	D3DXMATRIX rotationMatrix = xRot * yRot;
	D3DXVec3TransformCoord(&displacement, &displacement, &rotationMatrix);

	m_vecPos.x += displacement.x;
	m_vecPos.z += displacement.z;
	m_vecPos.y += displacement.y;

}