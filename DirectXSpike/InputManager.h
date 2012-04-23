#pragma once
/********************************************************************
	created:	2012/04/23
	filename: 	InputManager.h
	author:		Matthew Alford
	
	purpose:	Buffers raw input messages and translates to game key codes
*********************************************************************/

//////////////////////////////////////
// Includes
//////////////////////////////////////
#include "Keyboard.h"
#include "GameEvents.h"

//////////////////////////////////////
// Forward Declarations
//////////////////////////////////////
class CPlayer;
interface IGameState;

//////////////////////////////////////
// Class Definition
//////////////////////////////////////
class CInputManager
{
public:
	CInputManager(void);
	~CInputManager(void);

private:
	D3DXVECTOR2 m_bufferMouse;
	bool		m_bCameraAttachMode;
	IEventHandler *m_gameState;
	WORD		m_buttonBuffer;
	WORD		m_prevFrameButtonBuffer;
	int			m_bufferMouseWheel;

public:
	void Setup();
	void Update();
	
	inline void MouseMoved(int dX, int dY) {m_bufferMouse.x += (float)dX; m_bufferMouse.y += (float)dY;}	
	inline void MouseWheel(int mouseDelta) {m_bufferMouseWheel += mouseDelta;}
	inline void KeyDown( UINT keycode ) {UINT virtualkey = TranslateKey( keycode ); m_buttonBuffer |= (1<<(virtualkey-1));}
	inline void KeyUp( UINT keycode ) {UINT virtualkey = TranslateKey( keycode ); m_buttonBuffer &= ~(1<<(virtualkey-1));}
	inline D3DXVECTOR2 GetMousePos() {return m_bufferMouse;}
	inline void SetGameState(IEventHandler* gs) {m_gameState = gs;}

};