#pragma once
#include "gamestate.h"

//////////////////////////////////////
// Forward Declarations
//////////////////////////////////////
class CPlayer;
class CRenderEngine;
class CLevelManager;
class CSceneManager;
class CCamera;
class CCollisionManager;
class CTextureManager;

//////////////////////////////////////
// Class Definition
//////////////////////////////////////
class CMainGameState : public IGameState
{
public:
	CMainGameState(void);
	~CMainGameState(void);

	virtual bool HandleEvent( UINT eventId, void* data, UINT data_sz );

private:
	CPlayer					   *m_pPlayer;
	CLevelManager			   *m_pLevelMgr;
	CSceneManager			   *m_pSceneMgr;
	CCamera					   *m_pCamera;
	CCollisionManager		   *m_pCollision;
	int							m_texContext;
	CTextureManager*			m_pTextureMgr;

	bool Init( CRenderEngine *renderEngine );
	void Update( float elapsedMillis );
	void Destroy();
	void KeyUp( UINT vk );
	void KeyDown( UINT vk );
	void MouseMoved( D3DXVECTOR2 mouseDisplacement );
	void MouseWheel( int mouseDelta );
};
