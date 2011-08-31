#pragma once
//////////////////////////////////////////////////////////////////////////
// HUD.h - 2011 Matthew Alford
//////////////////////////////////////////////////////////////////////////

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
	bool					m_bShowLightDir;

	bool Setup();

public:
	void SetDevice(LPDIRECT3DDEVICE9 device);
	void Render();

	inline void SetCurrentPlayerPos(D3DXVECTOR3 pos) {m_currentPlayerPos = pos;}
	inline void SetCurrentLightDir(D3DXVECTOR3 dir) {m_currentLightDir = dir;}
	inline void SetCurrentMousePos(float x, float y) {m_currentMousePos = D3DXVECTOR2(x,y);}
	inline void SetCurrentFPS(float fps) {m_currentFPS = fps;}
	inline void DisplayLightDirection(bool show) {m_bShowLightDir = show;}
};