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

using namespace GameEvents;

//////////////////////////////////////
// Type Definitions
//////////////////////////////////////
typedef enum _EGameStates
{
	STATE_MAIN_MENU = 0x00FF,
	STATE_MAIN_GAME,
	STATE_PAUSED
} EGameStates;

//////////////////////////////////////
// Class Definition
//////////////////////////////////////
class CGameStateStack : public SingleThreadSingleton<CGameStateStack, CGameStateStack>, public IEventHandler
{
	friend class SingleThreadSingleton<CGameStateStack, CGameStateStack>;
private:
	CGameStateStack(void);
	~CGameStateStack(void);

public:
	void Update(float elapsedMillis );
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
