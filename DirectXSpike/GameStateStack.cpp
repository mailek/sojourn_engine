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
	mainGame->HandleEvent( EVT_INIT, &renderEngine, sizeof(CRenderEngine*) );
	m_stateStack.push_back( mainGame );
	
	return true;
}

void CGameStateStack::Update( LPDIRECT3DDEVICE9 device, float elapsedMillis )
{
	for( std::vector<IGameState*>::iterator it = m_stateStack.begin(), _it = m_stateStack.end(); it != _it; it++ )
	{
		(*it)->HandleEvent( EVT_UPDATE, &elapsedMillis, sizeof(float) );
	}

}

void CGameStateStack::PushNewState( const UINT stateId )
{
	IGameState* state = GetGameState( stateId );
	m_stateStack.push_back( state );
}

void CGameStateStack::PopCurrentState()
{
	IGameState* state = *m_stateStack.end();
	if(state)
	{
		state->HandleEvent( EVT_DESTROY, NULL, 0 );
		m_stateStack.pop_back();
	}
}

CPlayer* CGameStateStack::GetPlayer()
{
	CPlayer*	player = NULL;
	IGameState* state = m_stateStack.back();
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
		IGameState* state = m_stateStack.back();
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
	for( std::vector<IGameState*>::iterator it = m_stateStack.begin(), _it = m_stateStack.end(); it != _it; _it-- )
	{
		(*it)->HandleEvent( EVT_DESTROY, NULL, 0 );
	}
}

IGameState* CGameStateStack::GetGameState( const UINT stateId )
{
	return NULL;
}