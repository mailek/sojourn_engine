/********************************************************************
	created:	2012/04/23
	filename: 	Player.cpp
	author:		Matthew Alford
	
	purpose:	
*********************************************************************/
#include "StdAfx.h"
#include "Player.h"
#include "MeshManager.h"
#include "Terrain.h"
#include "mathutil.h"
#include "renderengine.h"
#include "camera.h"

//////////////////////////////////////
// Type Definitions
//////////////////////////////////////

typedef enum _EMovementFlagType
{
	MOVE_FORWARD_FLAG	= (1<<0), 
	MOVE_BACKWARD_FLAG	= (1<<1), 
	MOVE_LEFT_FLAG		= (1<<2), 
	MOVE_RIGHT_FLAG		= (1<<3),
	/* all flags */
	MOVE_FLAG_MASK		= MOVE_FORWARD_FLAG | MOVE_BACKWARD_FLAG | MOVE_LEFT_FLAG | MOVE_RIGHT_FLAG
} EMovementFlagType;

//////////////////////////////////////////////////////////////////////////
// Setup Functions
//////////////////////////////////////////////////////////////////////////

CPlayer::CPlayer(void) : m_pModel(NULL),
						m_movementState(0),
						m_pTerrain(NULL)
{
	::ZeroMemory(&m_properties, sizeof(m_properties));
	m_vecVelocity = Vector_3(0.0f, 0.0f, 0.0f);
	m_vecRotationVelocity = Vector_3(0.0f, 0.0f, 0.0f);

	m_properties.collideSphereRadius = 5.0f;
	m_properties.maxRotateSpeed = 2*PI; 
	collidableType = CHARACTER;
}

CPlayer::~CPlayer(void)
{
}

void CPlayer::Setup(CMeshManager& meshMgr)
{
	meshMgr.GetMesh(CMeshManager::eAnimTiny, &m_pModel);
	//meshMgr.GetMesh(CMeshManager::eMultiAnimTiny, &m_pModel);
	//meshMgr.GetMesh(CMeshManager::eCherryTreeLow, &m_pModel);
	SetPosition3D(Vector_3(0.0f, 0.0f, 0.0f));
}

//////////////////////////////////////////////////////////////////////////
// Update Functions
//////////////////////////////////////////////////////////////////////////

void CPlayer::Update(float deltaTime)
{
	::ZeroMemory(m_vecVelocity, sizeof(m_vecVelocity));
	if(m_movementState & MOVE_FORWARD_FLAG)
		m_vecVelocity.z += 150.0f;

	if(m_movementState & MOVE_BACKWARD_FLAG)
		m_vecVelocity.z -= 150.0f;

	float yRotation = m_transform.GetRotationEulers().y;
	if(m_movementState & MOVE_RIGHT_FLAG)
		yRotation += deltaTime * 2.0f;//		m_vecRotationVelocity.x += MAXROTATIONRATE;

	if(m_movementState & MOVE_LEFT_FLAG)
		yRotation -= deltaTime * 2.0f;//m_vecRotationVelocity.x -= MAXROTATIONRATE;

	m_transform.SetYRotationRadians(yRotation);

	// update animation states
	if(m_vecVelocity.z > 0.0f)
	{
		// walk forward
		m_pModel->SetAnimation(0);

	} else if(m_vecVelocity.z < 0.0f)
	{
		// walk backward
		m_pModel->SetAnimation(1);
	} else if( (m_movementState & (MOVE_RIGHT_FLAG | MOVE_LEFT_FLAG)) == MOVE_RIGHT_FLAG)
	{
		m_pModel->SetAnimation(2);
	} else if( (m_movementState & (MOVE_RIGHT_FLAG | MOVE_LEFT_FLAG)) == MOVE_LEFT_FLAG)
	{
		m_pModel->SetAnimation(3);
	} else
	{
		// stop animations
		m_pModel->SetAnimation(-1);
	}
	//else if(m_vecEuler.y > 0.0f)
	//{
	//	// right turn
	//} else if(m_vecEuler.y < 0.0f)
	//{
	//	// left turn
	//}

	m_vecLastPos = m_transform.GetTranslation();

	Vector_3 displacement = m_vecVelocity * deltaTime;
	D3DXMATRIX rotationMatrix;
	D3DXMatrixRotationY(&rotationMatrix, m_transform.GetRotationEulers().y);
	D3DXVec3TransformCoord(&displacement, &displacement, &rotationMatrix);

	Vector_3 position = m_transform.GetTranslation();
	position.x += displacement.x;
	position.z += displacement.z;
	m_transform.SetTranslation(position);

	GroundClampTerrain();
}

void CPlayer::GroundClampTerrain()
{
	if(m_pTerrain == NULL)
		return;

	Vector_3 pos = m_transform.GetTranslation();
	if(fcompare(pos.x, m_vecLastPos.x) && fcompare(pos.z, m_vecLastPos.z))
		return;

	pos.y = m_pTerrain->GetTerrainHeightAtXZ(pos.x, pos.z);
	m_transform.SetTranslation(pos);
}

void CPlayer::SetPosition3D(Vector_3 &pos) 
{
	m_transform.SetTranslation(pos); 
	m_vecLastPos = pos;
	m_vecLastPos.z += 1.01f; // change the position a little so it groundclamps
} 

bool CPlayer::HandleEvent( UINT eventId, void* data, UINT data_sz )
{
	static const EMovementFlagType movementMap[] = {
		MOVE_FORWARD_FLAG,
		MOVE_BACKWARD_FLAG,
		MOVE_LEFT_FLAG,
		MOVE_RIGHT_FLAG,
		};

	switch( eventId )
	{
	case EVT_UPDATE:
		assert( data_sz == sizeof(float) );
		assert( data != NULL );
		Update( *(float*)data );
		break;
	case EVT_INIT:
		assert( data_sz == sizeof(CMeshManager*) );
		assert( data != NULL );
		Setup( *(CMeshManager*)data );
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
//		SetXRotationRadians(arg->x);
		SetYRotationRadians(arg->y);
//		SetZRotationRadians(arg->z);
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
		((CCamera*)data)->SetLocalPosition(Vector_3(-5.0f, 12.0f, -20.0f));
		break;
	default:
		assert(false);
		break;
	}

	return true;
}

void CPlayer::HandleCollision( ICollidable* other )
{
	switch(other->collidableType)
	{
	case OBSTACLE:
		m_transform.SetTranslation(m_vecLastPos);
		break;
	default:
		break;
	}

}

//////////////////////////////////////////////////////////////////////////
// Render Functions
//////////////////////////////////////////////////////////////////////////

void CPlayer::Render( CRenderEngine &rndr )
{
	/* Debug: draw a small white sphere at the player's position */
	Sphere_PosRad s;
	s.radius = 0.15f;
	s.pos = m_transform.GetTranslation();
	s.pos.y += 0.5f * s.radius;
	ColorRGBA32 c;
	c.a = 1.0f;
	c.r = 0.3f;
	c.g = 0.3f;
	c.b = 1.0f;
	rndr.DrawDebugSphere(s, c);

	/* Draw Player Model */
	m_pModel->SetRenderFunc(m_pModel->RenderFuncs.lightAndTexture);
	m_pModel->Render( rndr, GetWorldTransform(), rndr.GetShaderManager() );
}

D3DXMATRIX CPlayer::GetWorldTransform(void)
{
	const float LEANAMOUNT = 0.3f;
	D3DXMATRIX worldMatrix = m_transform.GetTransform();
	//D3DXMATRIX rotate;
	//D3DXMatrixIdentity(&rotate);
	//D3DXMatrixRotationX(&rotate, m_transform.GetRotationEulers().x*LEANAMOUNT); // take a portion of the look angle and make the player model lean forwards and back
	//worldMatrix *= rotate;

	/*D3DXMatrixIdentity(&rotate);
	D3DXMatrixRotationY(&rotate, m_vecEuler.y);
	worldMatrix *= rotate;
	*/
	//D3DXMatrixRotationZ(&rotate, m_veccEuler.z);
	//worldMatrix *= rotate;*/

	return worldMatrix;
}