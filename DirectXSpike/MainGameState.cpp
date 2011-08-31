#include "StdAfx.h"
#include "MainGameState.h"
#include "RenderEngine.h"
#include "Player.h"
#include "Skybox.h"
#include "Terrain.h"
#include "Keyboard.h"
#include "mathutil.h"

CMainGameState::CMainGameState(void) :  m_pPlayer(NULL),
										m_pLevelMgr(NULL),
										m_pSceneMgr(NULL),
										m_pCamera(NULL)
{
}

CMainGameState::~CMainGameState(void)
{
}

bool CMainGameState::HandleEvent( UINT eventId, void* data, UINT data_sz )
{
	switch( eventId )
	{
	case EVT_GETPLAYER:
		assert( data_sz == sizeof(void*) );
		assert( data != NULL );
		*(CPlayer**)data = m_pPlayer;
		break;
	case EVT_UPDATE:
		assert( data_sz == sizeof(float) );
		assert( data != NULL );
		Update( *(float*)data );
		break;
	case EVT_RENDER:
		break;
	case EVT_INIT:
		assert( data_sz == sizeof(CRenderEngine*) );
		assert( data != NULL );
		Init( *(CRenderEngine**)data );
		break;
	case EVT_KEYDOWN:
		assert( data_sz == sizeof(UINT) );
		assert( data != NULL );
		KeyDown( *(UINT*)data );
		break;
	case EVT_KEYUP:
		assert( data_sz == sizeof(UINT) );
		assert( data != NULL );
		KeyUp( *(UINT*)data );
		break;
	case EVT_MOUSE_MOVED:
		assert( data_sz == sizeof(D3DXVECTOR2) );
		assert( data != NULL );
		MouseMoved( *(D3DXVECTOR2*)data );
		break;
	case EVT_MOUSE_WHEEL:
		assert( data_sz == sizeof(int) );
		assert( data != NULL );
		MouseWheel( *(int*)data );
		break;
	}

	return true;
}

bool CMainGameState::Init( CRenderEngine *renderEngine )
{
	LPDIRECT3DDEVICE9		device = renderEngine->GetDevice();
	CMeshManager*			meshMgr = &renderEngine->GetMeshManager();

	m_pLevelMgr = new CLevelManager();
	m_pSceneMgr = &renderEngine->GetSceneManager();
	m_pCamera = &m_pSceneMgr->GetDefaultCamera();

	// level objects
	CTerrain*				pTerrain;
	CSkybox*				pSkyDome;
	if(!m_pLevelMgr->LoadDefaultLevel( renderEngine, &pTerrain, &pSkyDome ))
	{
		PTR_SAFEDELETE(m_pLevelMgr);
		return false;	
	}

	// player
	m_pPlayer = new CPlayer();
	m_pPlayer->Setup(*meshMgr);
	m_pSceneMgr->AddNonclippableObjectToScene(m_pPlayer);
	
	// camera
	//m_pSceneMgr->SetCamera(camera);
	m_pCamera->Set3DPosition(D3DXVECTOR3(0.0f, 10.0f, -24.0f));
	m_pCamera->SetPlayerForFollow(m_pPlayer);
	m_pCamera->SetTerrainToClamp(pTerrain);
	pSkyDome->SetCameraObjectToFollow(m_pCamera);

	// terrain clamping
	m_pPlayer->SetGroundClampTerrain(*pTerrain);

	return true;
}

void CMainGameState::Update( float elapsedMillis )
{
	// send update msg  
	if(m_pPlayer)
	{
		m_pPlayer->Update(elapsedMillis);
	}
}

void CMainGameState::KeyUp( UINT vk )
{
	switch( vk )
	{
		case VGK_DIRUP:
			m_pPlayer->ClearMovementState(MOVE_FORWARD);
			break;
		case VGK_DIRLEFT:
			m_pPlayer->ClearMovementState(MOVE_LEFT);
			break;
		case VGK_DIRDOWN:
			m_pPlayer->ClearMovementState(MOVE_BACKWARD);
			break;
		case VGK_DIRRIGHT:
			m_pPlayer->ClearMovementState(MOVE_RIGHT);
			break;
		default:
			break;
	}
}

void CMainGameState::KeyDown( UINT vk )
{
	switch( vk )
	{
		case VGK_DIRUP:
			m_pPlayer->SetMovementState(MOVE_FORWARD);
			break;
		case VGK_DIRLEFT:
			m_pPlayer->SetMovementState(MOVE_LEFT);
			break;
		case VGK_DIRDOWN:
			m_pPlayer->SetMovementState(MOVE_BACKWARD);
			break;
		case VGK_DIRRIGHT:
			m_pPlayer->SetMovementState(MOVE_RIGHT);
			break;
		default:
			break;
	}
}
void CMainGameState::MouseMoved( D3DXVECTOR2 mouseDisplacement )
{
	if( m_pPlayer )
	{
		D3DXVECTOR3 playerRotation = m_pPlayer->GetRotationRadians();
		
		static const float halfPi = D3DX_PI/2.0f;
		playerRotation += D3DXVECTOR3(mouseDisplacement.y, mouseDisplacement.x, 0); // x mouse movement rotates look about Y axis, y mouse movement rotates the camera and lookat pitch about the X axis
		playerRotation.x = fclamp(playerRotation.x, -halfPi+.001f, halfPi-.001f);
		m_pPlayer->SetYRotationRadians(playerRotation.y);
		m_pPlayer->SetXRotationRadians(playerRotation.x);
	}
}

void CMainGameState::MouseWheel( int mouseDelta )
{
	if(m_pCamera)
	{
		const float ZOOM_SPEED = 0.0004f;
		m_pCamera->AdjustFOVAngle(mouseDelta*ZOOM_SPEED);
	}
}