/********************************************************************
	created:	2012/04/16
	filename: 	MainGameState.cpp
	author:		Matthew Alford
	
	purpose:	
*********************************************************************/
#include "StdAfx.h"
#include "MainGameState.h"
#include "RenderEngine.h"
#include "Player.h"
#include "Skybox.h"
#include "TerrainPatch.h"
#include "Keyboard.h"
#include "collisionmanager.h"
#include "texturemanager.h"

//////////////////////////////////////////////////////////////////////////
// Setup Functions
//////////////////////////////////////////////////////////////////////////

CMainGameState::CMainGameState(void) :  m_pPlayer(NULL),
										m_pLevelMgr(NULL),
										m_pSceneMgr(NULL),
										m_pTextureMgr(NULL),
										m_pCamera(NULL),
										m_pAvatar(NULL),
										m_pCollision(NULL),
										m_texContext(0)
{
}

CMainGameState::~CMainGameState(void)
{
}

/************************************************
*   Name:   CMainGameState::Init
*   Desc:   
************************************************/
void CMainGameState::Init( CRenderEngine *renderEngine )
{
	m_pLevelMgr = new CLevelManager();
	m_pSceneMgr = renderEngine->GetSceneManager();
	m_pCamera = &m_pSceneMgr->GetDefaultCamera();
	m_pCollision = CCollisionManager::GetInstance();

	m_pTextureMgr = &renderEngine->GetTextureManager();
	m_texContext = m_pTextureMgr->CreateTextureContext("MainGame");

	// level objects
	m_pLevelMgr->LoadDefaultLevel(renderEngine);
	m_pLevelMgr->RegisterStaticCollision(m_pCollision);

	// player
	m_pPlayer = new CPlayer();
	m_pPlayer->Setup(renderEngine->GetMeshManager());
	m_pSceneMgr->AddNonclippableObjectToScene(m_pPlayer);
	m_pCollision->RegisterDynamicCollidable(m_pPlayer);

	// camera
	//m_pSceneMgr->SetCamera(camera);
	SetAvatar(m_pPlayer);
	m_pCamera->SetTerrainToClamp(m_pLevelMgr->GetTerrain());
	m_pPlayer->SetGroundClampTerrain(m_pLevelMgr->GetTerrain());

	// terrain clamping
	//m_pCollision->RegisterStaticCollidable(pTerrain); // this should be handled now in levelmgr:registerstaticcollision

	return;
}

/************************************************
*   Name:   CMainGameState::Destroy
*   Desc:   
************************************************/
void CMainGameState::Destroy()
{
	/* delete dynamically created objects owned by the state */
	ptr_safedelete(m_pLevelMgr);
	ptr_safedelete(m_pPlayer);

	/* unregister all collidables */
	m_pCollision->UnregisterCollidable(m_pPlayer);

	/* destroy the collision module */
	CCollisionManager::DestroySingleton();

	/* unload all textures owned by the state */
	if(m_pTextureMgr) m_pTextureMgr->UnloadTextureContext(m_texContext);
}

//////////////////////////////////////////////////////////////////////////
// Update Functions
//////////////////////////////////////////////////////////////////////////

/************************************************
*   Name:   CMainGameState::Update
*   Desc:   
************************************************/
void CMainGameState::Update( float elapsedMillis )
{
	/* update dynamic objects */
	if(m_pPlayer) m_pPlayer->HandleEvent(EVT_UPDATE, &elapsedMillis, sizeof(elapsedMillis));

	/* check collisions */
	if(m_pCollision) m_pCollision->Update();

	/* update camera */
	m_cameraGhost.HandleEvent(EVT_UPDATE, &elapsedMillis, sizeof(elapsedMillis));
	if(m_pSceneMgr) m_pSceneMgr->GetDefaultCamera().Update(elapsedMillis);

	/* update level manager */
	if(m_pLevelMgr)	m_pLevelMgr->HandleEvent(EVT_UPDATE, &elapsedMillis, sizeof(elapsedMillis));
}

/************************************************
*   Name:   CMainGameState::HandleEvent
*   Desc:   
************************************************/
bool CMainGameState::HandleEvent( UINT eventId, void* data, UINT data_sz )
{
	switch( eventId )
	{
	case EVT_DEBUGCAMERA:
		{
		assert( data_sz == sizeof(bool) );
		assert( data != NULL );
		IEventHandler* avatar = (*(bool*)data) ? (IEventHandler*)&m_cameraGhost : (IEventHandler*)m_pPlayer;
		SetAvatar(avatar);
		break;
		}
	case EVT_GETAVATAR:
		assert( data_sz == sizeof(GetAvatarEventArgType) );
		assert( data != NULL );
		*(GetAvatarEventArgType)data = m_pAvatar;
		break;
	case EVT_UPDATE:
		assert( data_sz == sizeof(float) );
		assert( data != NULL );
		Update( *(float*)data );
		break;
	case EVT_DESTROY:
		Destroy();
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
	case EVT_GETTEXCONTEXT:
		assert( data_sz == sizeof(TextureContextIdType) );
		assert( data != NULL );
		*(TextureContextIdType*)data = m_texContext;
		break;
	default:
		assert(false);

	}

	return true;
}

/************************************************
*   Name:   CMainGameState::KeyUp
*   Desc:   
************************************************/
void CMainGameState::KeyUp( UINT vk )
{
	if(!m_pAvatar) 
		return;

	MovementCommandType arg;
	arg.clearFlag = true;

	switch( vk )
	{
		case VGK_DIRUP:
			arg.state = MOVE_FORWARD;
			break;
		case VGK_DIRLEFT:
			arg.state = MOVE_LEFT;
			break;
		case VGK_DIRDOWN:
			arg.state = MOVE_BACKWARD;
			break;
		case VGK_DIRRIGHT:
			arg.state = MOVE_RIGHT;
			break;
		default:
			assert(false);
			break;
	}

	m_pAvatar->HandleEvent(EVT_MOVECMD, &arg, sizeof(arg));
}

/************************************************
*   Name:   CMainGameState::KeyDown
*   Desc:   
************************************************/
void CMainGameState::KeyDown( UINT vk )
{
	if(!m_pAvatar) 
		return;

	MovementCommandType arg;
	arg.clearFlag = false;

	switch( vk )
	{
		case VGK_DIRUP:
			arg.state = MOVE_FORWARD;
			break;
		case VGK_DIRLEFT:
			arg.state = MOVE_LEFT;
			break;
		case VGK_DIRDOWN:
			arg.state = MOVE_BACKWARD;
			break;
		case VGK_DIRRIGHT:
			arg.state = MOVE_RIGHT;
			break;
		default:
			assert(false);
			break;
	}

	m_pAvatar->HandleEvent(EVT_MOVECMD, &arg, sizeof(arg));
}

/************************************************
*   Name:   CMainGameState::MouseMoved
*   Desc:   
************************************************/
void CMainGameState::MouseMoved( D3DXVECTOR2 mouseDisplacement )
{
	if(!m_pAvatar)
		return;

    D3DXVECTOR3 avatarRotation;
	m_pAvatar->HandleEvent(EVT_GETFACINGVEC, &avatarRotation, sizeof(avatarRotation));
	
	/* x mouse movement rotates look about Y axis, 
	   y mouse movement rotates the camera and lookat pitch about the X axis */
	static const float halfPi = D3DX_PI/2.0f;
	avatarRotation += D3DXVECTOR3(mouseDisplacement.y, mouseDisplacement.x, 0);
	avatarRotation.x = fclamp(avatarRotation.x, -halfPi+.001f, halfPi-.001f);

	m_pAvatar->HandleEvent(EVT_SETFACINGVEC, &avatarRotation, sizeof(avatarRotation));
}

/************************************************
*   Name:   CMainGameState::MouseWheel
*   Desc:   
************************************************/
void CMainGameState::MouseWheel( int mouseDelta )
{
	if(!m_pCamera) 
		return;
	
	static const float ZOOM_SPEED = 0.0004f;
	m_pCamera->AdjustFOVAngle(mouseDelta*ZOOM_SPEED);
}

/************************************************
*   Name:   CMainGameState::SetAvatar
*   Desc:   
************************************************/
void CMainGameState::SetAvatar(IEventHandler* avatar)
{
	IEventHandler* old = m_pAvatar;
	m_pAvatar = avatar;

	if(old)	old->HandleEvent(EVT_CONTROL_LOST_FOCUS, NULL, 0);
	if(m_pAvatar) m_pAvatar->HandleEvent(EVT_CONTROL_GAIN_FOCUS, NULL, 0);
	if(m_pCamera) m_pCamera->SetCameraAttach(m_pAvatar);
	if(m_pAvatar) m_pAvatar->HandleEvent(EVT_ATTACH_CAMERA, m_pCamera, sizeof(m_pCamera));
	if(m_pLevelMgr) m_pLevelMgr->HandleEvent(EVT_SETAVATAR, avatar, sizeof(avatar));
}