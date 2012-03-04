#pragma once
//////////////////////////////////////////////////////////////////////////
// GameStateStack.h - 2011 Matthew Alford
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////
// Includes
//////////////////////////////////////
#include "GameEvents.h"
#include "doublelinkedlist.h"
#include "singleton.h"

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
class CGameStateStack : public SingleThreadSingleton<CGameStateStack>, public IEventHandler
{
	friend class SingleThreadSingleton<CGameStateStack>;
private:
	CGameStateStack(void);
	~CGameStateStack(void);

public:
	void Update( LPDIRECT3DDEVICE9 device, float elapsedMillis );
	void PushNewState( const UINT stateId );
	void PopCurrentState();
	IEventHandler* GetCurrentState();
	bool Init( CRenderEngine *renderEngine );
	IEventHandler* GetAvatar();
	void ShutDown();

	virtual bool HandleEvent( UINT eventId, void* data, UINT data_sz );

private:
	CDoubleLinkedList<IEventHandler>    m_stateStack;

	IEventHandler* GetGameState( const UINT stateId );
};
