#pragma once
/********************************************************************
	created:	2012/04/23
	filename: 	HUD.h
	author:		Matthew Alford
	
	purpose:	
*********************************************************************/

//////////////////////////////////////
// Class Definition
//////////////////////////////////////
class CHUD
{
public:
	CHUD(void);
	~CHUD(void);

private:
	LPDIRECT3DDEVICE9		m_device;
	LPD3DXFONT				m_debugFont;
	D3DXVECTOR3				m_currentPlayerPos;
	D3DXVECTOR3				m_currentLightDir;
	D3DXVECTOR2				m_currentMousePos;
	float					m_currentFPS;

	bool Setup();

public:
	void SetDevice(LPDIRECT3DDEVICE9 device);
	void Render();

	inline void SetCurrentAvatarPos(D3DXVECTOR3 pos) {m_currentPlayerPos = pos;}
	inline void SetCurrentLightDir(D3DXVECTOR3 dir) {m_currentLightDir = dir;}
	inline void SetCurrentMousePos(float x, float y) {m_currentMousePos = D3DXVECTOR2(x,y);}
	inline void SetCurrentFPS(float fps) {m_currentFPS = fps;}
};