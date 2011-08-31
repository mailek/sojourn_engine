#include "StdAfx.h"
#include "HUD.h"

//////////////////////////////////////////////////////////////////////////
// Setup Functions
//////////////////////////////////////////////////////////////////////////

CHUD::CHUD(void) : m_debugFont(NULL),
					m_currentPlayerPos(D3DXVECTOR3(0,0,0)),
					m_currentFPS(0.0f),
					m_bShowLightDir(false)
{
}

CHUD::~CHUD(void)
{
	COM_SAFERELEASE(m_debugFont);
}

void CHUD::SetDevice(LPDIRECT3DDEVICE9 device)
{
	m_device = device;
	Setup();
}

bool CHUD::Setup()
{
	::ZeroMemory(&m_currentMousePos, sizeof(m_currentMousePos));

	// Create the debug font
	D3DXFONT_DESC fontDesc;
	::ZeroMemory(&fontDesc, sizeof(fontDesc));
	fontDesc.Height				= 30;
	fontDesc.Width				= 12;
	fontDesc.Weight				= 500;
	fontDesc.Italic				= false;
	fontDesc.CharSet			= DEFAULT_CHARSET;
	fontDesc.OutputPrecision	= OUT_DEFAULT_PRECIS;
	fontDesc.Quality			= DEFAULT_QUALITY;
	fontDesc.PitchAndFamily		= DEFAULT_PITCH | FF_DONTCARE;
	strcpy_s(fontDesc.FaceName, "Helvetica");
	HR( D3DXCreateFontIndirect(m_device, &fontDesc, &m_debugFont) );

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Render Functions
//////////////////////////////////////////////////////////////////////////

void CHUD::Render()
{
	// Draw the FPS Counter
	RECT rect;
	rect.top = 25; rect.left = 25;
	rect.bottom	= 100; rect.right = 300;
	char szOutput[50];

	sprintf_s(szOutput, "FPS: %.0f", m_currentFPS);
	m_debugFont->DrawTextA(0, szOutput, -1, &rect, DT_TOP | DT_LEFT, D3DCOLOR_ARGB(255, 0, 0, 0));
	rect.top++;	rect.left++;
	m_debugFont->DrawTextA(0, szOutput, -1, &rect, DT_TOP | DT_LEFT, D3DCOLOR_ARGB(255,255,220,200));

	// Debug Player Position
	rect.top = 50; rect.left = 25;
	rect.bottom	= 100; rect.right = 500;

	sprintf_s(szOutput, "Player Pos: %.2f, %.2f, %.2f", m_currentPlayerPos.x, m_currentPlayerPos.y, m_currentPlayerPos.z);
	m_debugFont->DrawTextA(0, szOutput, -1, &rect, DT_TOP | DT_LEFT, D3DCOLOR_ARGB(255, 0, 0, 0));
	rect.top++; rect.left++;
	rect.bottom	= 100; rect.right = 500;
	m_debugFont->DrawTextA(0, szOutput, -1, &rect, DT_TOP | DT_LEFT, D3DCOLOR_ARGB(255,255,220,200));

	if(m_bShowLightDir)
	{
		// Debug Light Position
		rect.top = 75; rect.left = 25;
		rect.bottom	= 100; rect.right = 500;

		sprintf_s(szOutput, "Light Dir: %.2f, %.2f, %.2f", m_currentLightDir.x, m_currentLightDir.y, m_currentLightDir.z);
		m_debugFont->DrawTextA(0, szOutput, -1, &rect, DT_TOP | DT_LEFT, D3DCOLOR_ARGB(255, 0, 0, 0));
		rect.top++; rect.left++;
		rect.bottom	= 100; rect.right = 500;
		m_debugFont->DrawTextA(0, szOutput, -1, &rect, DT_TOP | DT_LEFT, D3DCOLOR_ARGB(255,255,220,200));
	}

	// Debug Mouse Position
	rect.top = 100; rect.left = 25;
	rect.bottom	= 150; rect.right = 500;

	sprintf_s(szOutput, "Mouse Pos: %d, %d", m_currentMousePos[0], m_currentMousePos[1]);
	m_debugFont->DrawTextA(0, szOutput, -1, &rect, DT_TOP | DT_LEFT, D3DCOLOR_ARGB(255, 0, 0, 0));
	rect.top++; rect.left++;
	rect.bottom	= 150; rect.right = 500;
	m_debugFont->DrawTextA(0, szOutput, -1, &rect, DT_TOP | DT_LEFT, D3DCOLOR_ARGB(255,255,220,200));
}