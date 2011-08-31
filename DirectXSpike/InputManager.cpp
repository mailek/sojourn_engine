#include "StdAfx.h"
#include "InputManager.h"
#include "Player.h"
#include "GameState.h"

#define MOUSE_DEAD_ZONE		(0.001f)

static const D3DXVECTOR2	s_mouseSpeed(0.005f, 0.003f);

//////////////////////////////////////////////////////////////////////////
// Setup Functions
//////////////////////////////////////////////////////////////////////////

CInputManager::CInputManager(void) : m_bufferMouse(D3DXVECTOR2(0,0)),
									m_bCameraAttachMode(true),
									m_gameState(NULL),
									m_buttonBuffer(0),
									m_prevFrameButtonBuffer(0),
									m_bufferMouseWheel(0)
{
}

CInputManager::~CInputManager(void)
{

}

void CInputManager::Setup()
{
	
}

//////////////////////////////////////////////////////////////////////////
// Update Functions
//////////////////////////////////////////////////////////////////////////

void CInputManager::Update()
{
	// update mouse and keyboard button changes
	if(m_gameState)
	{
		// update mouse movement from buffer if greater than dead zone cutoff
		if( (fabs(m_bufferMouse.x ) > MOUSE_DEAD_ZONE) || (fabs(m_bufferMouse.y) > MOUSE_DEAD_ZONE) )
		{
			D3DXVECTOR2 mouse(m_bufferMouse.x * s_mouseSpeed.x, m_bufferMouse.y * s_mouseSpeed.y);
			m_gameState->HandleEvent( EVT_MOUSE_MOVED, &mouse, sizeof(D3DXVECTOR2) );
		}
	
		// handle button state changes
		WORD changedButtons = m_buttonBuffer ^ m_prevFrameButtonBuffer;
		WORD keyDownChanges = changedButtons & m_buttonBuffer;
		WORD keyUpChanges = changedButtons & (~m_buttonBuffer);

		WORD buttonMask = 1;
		for( UINT i = 1; i <= sizeof(WORD)*BITS_PER_BYTE; i++, buttonMask <<= 1 )
		{
			if( (keyUpChanges & buttonMask) != 0 )
			{
				m_gameState->HandleEvent( EVT_KEYUP, &i, sizeof(UINT) );
			}
			else if( (keyDownChanges & buttonMask) != 0 )
			{
				m_gameState->HandleEvent( EVT_KEYDOWN, &i, sizeof(UINT) );
			}

		}

		if(m_bufferMouseWheel !=0 )
			m_gameState->HandleEvent( EVT_MOUSE_WHEEL, &m_bufferMouseWheel, sizeof(int) );

	}

	// save the button states for next frame
	m_prevFrameButtonBuffer = m_buttonBuffer;

	// reset the mouse buffer
	if(m_bCameraAttachMode)
	{
		m_bufferMouse = D3DXVECTOR2(0,0);
	}

	m_bufferMouseWheel = 0;
}