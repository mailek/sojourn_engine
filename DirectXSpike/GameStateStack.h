#pragma once
//////////////////////////////////////////////////////////////////////////
// GameStateStack.h - 2011 Matthew Alford
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////
// Includes
//////////////////////////////////////
#include "GameState.h"
#include "doublelinkedlist.h"

//////////////////////////////////////
// Forward Declarations
//////////////////////////////////////
class CRenderEngine;
class CPlayer;

//////////////////////////////////////
// Type Definitions
//////////////////////////////////////
enum {
	STATE_MAIN_MENU = 0x00FF,
	STATE_MAIN_GAME,
	STATE_PAUSED
};

//////////////////////////////////////
// Class Definition
//////////////////////////////////////
class CGameStateStack : public IGameState
{
public:
	CGameStateStack(void);
	~CGameStateStack(void);

	void Update( LPDIRECT3DDEVICE9 device, float elapsedMillis );
	void PushNewState( const UINT stateId );
	void PopCurrentState();
	IGameState* GetCurrentState();
	bool Init( CRenderEngine *renderEngine );
	CPlayer* GetPlayer();
	void ShutDown();

	virtual bool HandleEvent( UINT eventId, void* data, UINT data_sz );

private:
	CDoubleLinkedList<IGameState>    m_stateStack;

	IGameState* GetGameState( const UINT stateId );
};
