#include "StdAfx.h"
#include "Player.h"
#include "MeshManager.h"
#include "Terrain.h"
#include "mathutil.h"
#include "renderengine.h"

//////////////////////////////////////////////////////////////////////////
// Setup Functions
//////////////////////////////////////////////////////////////////////////

CPlayer::CPlayer(void) : m_pModel(NULL),
						m_movementState(0),
						m_pTerrain(NULL)
{
	::ZeroMemory(m_vecPos, sizeof(m_vecPos));
	::ZeroMemory(m_vecEuler, sizeof(m_vecEuler));
	::ZeroMemory(&m_properties, sizeof(m_properties));
	m_vecVelocity = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	m_vecRotationVelocity = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	m_vecScale = D3DXVECTOR3(1.0f, 1.0f, 1.0f);

	m_properties.collideSphereRadius = 5.0f;
	m_properties.maxRotateSpeed = 2*D3DX_PI; 
}

CPlayer::~CPlayer(void)
{
}

void CPlayer::Setup(CMeshManager& meshMgr)
{
	//meshMgr.GetMesh(CMeshManager::eTinyX, &m_pMesh);
	meshMgr.GetMesh(CMeshManager::eAnimTiny, &m_pModel);
	//meshMgr.GetMesh(CMeshManager::eMultiAnimTiny, &m_pModel);
	//meshMgr.GetMesh(CMeshManager::eCherryTreeLow, &m_pModel);
	//meshMgr.GetMesh(CMeshManager::eTeapot, &m_pModel);
	SetPosition3D(D3DXVECTOR3(0.0f, 0.0f, 0.0f));
}

//////////////////////////////////////////////////////////////////////////
// Update Functions
//////////////////////////////////////////////////////////////////////////

void CPlayer::ClearMovementState(int state)
{
	DWORD movementFlag(0);
	switch(state)
	{
		case MOVE_FORWARD:
			m_movementState &= ~MOVE_FORWARD;
			break;
		case MOVE_BACKWARD:
			m_movementState &= ~MOVE_BACKWARD;
			break;
		case MOVE_LEFT:
			m_movementState &= ~MOVE_LEFT;
			break;
		case MOVE_RIGHT:
			m_movementState &= ~MOVE_RIGHT;
			break;
	}
}

void CPlayer::SetMovementState(int state)
{
	DWORD movementFlag(0);
	switch(state)
	{
		case MOVE_FORWARD:
			m_movementState |= MOVE_FORWARD;
			break;
		case MOVE_BACKWARD:
			m_movementState |= MOVE_BACKWARD;
			break;
		case MOVE_LEFT:
			m_movementState |= MOVE_LEFT;
			break;
		case MOVE_RIGHT:
			m_movementState |= MOVE_RIGHT;
			break;
	}
}

//////////////////////////////////////////////////////////////////////////
// Render Functions
//////////////////////////////////////////////////////////////////////////

void CPlayer::Render( CRenderEngine &rndr )
{
	m_pModel->Render( rndr.GetDevice(), GetWorldTransform(), rndr.GetShaderManager() );
}


D3DXMATRIX CPlayer::GetWorldTransform(void)
{
	const float LEANAMOUNT = 0.3f;
	D3DXMATRIX worldMatrix;
	D3DXMatrixIdentity(&worldMatrix);

	D3DXMATRIX scale;
	D3DXMatrixScaling(&scale, m_vecScale.x, m_vecScale.y, m_vecScale.z);
	worldMatrix *= scale;

	D3DXMATRIX rotate;
	D3DXMatrixIdentity(&rotate);
	D3DXMatrixRotationX(&rotate, m_vecEuler.x*LEANAMOUNT); // take a portion of the look angle and make the player model lean forwards and back
	worldMatrix *= rotate;

	D3DXMatrixIdentity(&rotate);
	D3DXMatrixRotationY(&rotate, m_vecEuler.y);
	worldMatrix *= rotate;
	
	//D3DXMatrixRotationZ(&rotate, m_vecEuler.z);
	//worldMatrix *= rotate;

	D3DXMATRIX translate;
	D3DXMatrixTranslation(&translate, m_vecPos.x, m_vecPos.y, m_vecPos.z);
	worldMatrix *= translate;

	return worldMatrix;
}

//////////////////////////////////////////////////////////////////////////
// Update Functions
//////////////////////////////////////////////////////////////////////////

void CPlayer::Update(float deltaTime)
{
	::ZeroMemory(m_vecVelocity, sizeof(m_vecVelocity));
	if(m_movementState & MOVE_FORWARD)
		m_vecVelocity.z += 150.0f;

	if(m_movementState & MOVE_BACKWARD)
		m_vecVelocity.z -= 150.0f;

	if(m_movementState & MOVE_RIGHT)
		m_vecEuler.y += deltaTime * 2.0f;//		m_vecRotationVelocity.x += MAXROTATIONRATE;

	if(m_movementState & MOVE_LEFT)
		m_vecEuler.y -= deltaTime * 2.0f;//m_vecRotationVelocity.x -= MAXROTATIONRATE;

	// update animation states
	if(m_vecVelocity.z > 0.0f)
	{
		// walk forward
		m_pModel->SetAnimation(0);

	} else if(m_vecVelocity.z < 0.0f)
	{
		// walk backward
		m_pModel->SetAnimation(1);
	} else if( (m_movementState & (MOVE_RIGHT | MOVE_LEFT)) == MOVE_RIGHT)
	{
		m_pModel->SetAnimation(2);
	} else if( (m_movementState & (MOVE_RIGHT | MOVE_LEFT)) == MOVE_LEFT)
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

	m_vecLastPos = m_vecPos;

	D3DXVECTOR3 displacement = m_vecVelocity * deltaTime;
	D3DXMATRIX rotationMatrix;
	D3DXMatrixRotationY(&rotationMatrix, m_vecEuler.y);
	D3DXVec3TransformCoord(&displacement, &displacement, &rotationMatrix);

	m_vecPos.x += displacement.x;
	m_vecPos.z += displacement.z;

	GroundClampTerrain();
}

void CPlayer::GroundClampTerrain()
{
	if(m_pTerrain == NULL)
		return;

	if(fcompare(m_vecPos.x, m_vecLastPos.x) && fcompare(m_vecPos.z, m_vecLastPos.z))
		return;

	m_vecPos.y = m_pTerrain->GetTerrainHeightAtXZ(m_vecPos.x, m_vecPos.z);
}

void CPlayer::SetPosition3D(D3DXVECTOR3 &pos) 
{
	m_vecPos = pos; 
	m_vecLastPos = pos; 
	m_vecLastPos.z += 1.01f; // change the position a little so it groundclamps
} 