#include "StdAfx.h"
#include "GameStateStack.h"
#include "LevelManager.h"
#include "SceneManager.h"
#include "MainGameState.h"

CGameStateStack::CGameStateStack(void)
{
}

CGameStateStack::~CGameStateStack(void)
{
	
}

bool CGameStateStack::Init( CRenderEngine *renderEngine )
{
	CMainGameState *mainGame = new CMainGameState();
	m_stateStack.AddItemToEnd( mainGame );
	mainGame->HandleEvent( EVT_INIT, &renderEngine, sizeof(CRenderEngine*) );
	
	return true;
}

void CGameStateStack::Update( LPDIRECT3DDEVICE9 device, float elapsedMillis )
{
	for( CDoubleLinkedList<IGameState>::DoubleLinkedListItem* it = m_stateStack.first; it != NULL; it = it->next )
	{
		it->item->HandleEvent( EVT_UPDATE, &elapsedMillis, sizeof(float) );
	}

}

void CGameStateStack::PushNewState( const UINT stateId )
{
	IGameState* state = GetGameState( stateId );
	m_stateStack.AddItemToEnd( state );
}

IGameState* CGameStateStack::GetCurrentState()
{
	IGameState* ret = NULL;
	CDoubleLinkedList<IGameState>::DoubleLinkedListItem* i = m_stateStack.last;
	if(i != NULL)
	{
		ret = i->item;
	}

	return ret;
}

void CGameStateStack::PopCurrentState()
{
	IGameState* state = GetCurrentState();
	if(state)
	{
		state->HandleEvent( EVT_DESTROY, NULL, 0 );
		m_stateStack.RemoveItem(state);
	}
}

CPlayer* CGameStateStack::GetPlayer()
{
	CPlayer*	player = NULL;
	
	IGameState* state = GetCurrentState();
	if( state )
	{
		state->HandleEvent( EVT_GETPLAYER, &player, sizeof(void*) );
	}

	return player;
}

bool CGameStateStack::HandleEvent( UINT eventId, void* data, UINT data_sz )
{
	switch(eventId)
	{
	case EVT_MOUSE_MOVED:
	case EVT_MOUSE_WHEEL:
	case EVT_KEYDOWN:
	case EVT_KEYUP:
		{
		// forward event to top state on stack
		IGameState* state = GetCurrentState();
		if( state )
		{
			state->HandleEvent( eventId, data, data_sz );
		}
		break;
		}
	default:
		assert(false);
		break;
	}
	return true;
}

void CGameStateStack::ShutDown()
{
	for( CDoubleLinkedList<IGameState>::DoubleLinkedListItem* it = m_stateStack.first; it != NULL; it = it->next )
	{
		it->item->HandleEvent( EVT_DESTROY, NULL, 0 );
	}
}

IGameState* CGameStateStack::GetGameState( const UINT stateId )
{
	return NULL;
}