#include "stdafx.h"
//////////////////////////////////////////////////////////////////////////
// DirectXSpike.cpp : Defines the entry point for the application.
// 2011 Matthew Alford
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////
// Includes
//////////////////////////////////////
#include "RenderEngine.h"
#include "HUD.h"
#include "Keyboard.h"
#include "InputManager.h"
#include "GameStateStack.h"
#include "Player.h"
#include "Settings.h"

//////////////////////////////////////
// Global Constants
//////////////////////////////////////
#define	MAX_LOADSTRING		(100)
static const int			WIDTH = SCREEN_RES_H;
static const int			HEIGHT = SCREEN_RES_V;
static const int			MAXBLURS = 4;
static const int			MAX_WIN_MSGS_PER_FRAME = 20;

//////////////////////////////////////
// Type Definitions
//////////////////////////////////////
struct APPLICATIONSTATE
{
	HINSTANCE		hInst;							// current instance
	TCHAR			szTitle[MAX_LOADSTRING];		// The title bar text
	TCHAR			szWindowClass[MAX_LOADSTRING];	// the main window class name
	HWND			hWnd;
	float			frameCount;
};

//////////////////////////////////////
// Local Variables
//////////////////////////////////////
//static CPlayer				s_player;
static CRenderEngine		s_renderEngine;
static CHUD					s_hud;
static CInputManager		s_inputMgr;
static APPLICATIONSTATE		m_appState;
static CGameStateStack		s_gameState;

//////////////////////////////////////
// Forward Declarations
//////////////////////////////////////
LRESULT CALLBACK WndProc
	(
	HWND hWnd, 
	UINT message, 
	WPARAM wParam, 
	LPARAM lParam
	);

void InitD3D
	(
	HINSTANCE hInstance, 
	int width, 
	int height,
	bool windowedMode,
	D3DDEVTYPE deviceType,
	HWND hwnd,
	IDirect3DDevice9** device
	);

bool Setup
	(
	LPDIRECT3DDEVICE9 device
	);

void Cleanup
	(
	void
	);

void Display
	(
	float elapsedmills, 
	LPDIRECT3DDEVICE9 device
	);

void Update
	(
	LPDIRECT3DDEVICE9 device, 
	float elapsedmills
	);

void CalculateFPS
	(
	float timeDelta
	);


//////////////////////////////////////
// Functions
//////////////////////////////////////
int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	MSG msg;
	::ZeroMemory(&m_appState, sizeof(m_appState));

	////////////////////////////
	// Make the window class
	////////////////////////////
	WNDCLASSEX	winClassEx;

#ifndef _DEBUG
	bool windowed = false;
#else
	bool windowed = true;
#endif

	winClassEx.cbSize			= sizeof(WNDCLASSEX);
	winClassEx.style			= CS_DBLCLKS | CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	winClassEx.lpfnWndProc		= WndProc;
	winClassEx.cbClsExtra		= 0;
	winClassEx.cbWndExtra		= 0;
	winClassEx.hInstance		= hInstance;
	winClassEx.hIcon			= NULL;//LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	winClassEx.hIconSm			= NULL;
	winClassEx.hCursor			= NULL; //LoadCursor(hInstance, NULL);
	winClassEx.hbrBackground	= windowed ? (HBRUSH)GetStockObject(BLACK_BRUSH) : NULL;
	winClassEx.lpszMenuName		= ""; 
	winClassEx.lpszClassName	= "DirectX_Spike";

	if(RegisterClassEx(&winClassEx) == 0)
		return -1;

	DWORD dwWindowStyleFlags = WS_VISIBLE;
	if(windowed)
		dwWindowStyleFlags |= WS_OVERLAPPEDWINDOW;
	else
		dwWindowStyleFlags |= WS_POPUP|WS_EX_TOPMOST;

	RECT rWindow;
	rWindow.left	= 0;
	rWindow.top		= 0;
	rWindow.right	= WIDTH;
	rWindow.bottom	= HEIGHT;

	AdjustWindowRectEx(&rWindow, 
		dwWindowStyleFlags,
		FALSE, 
		WS_EX_APPWINDOW);

	int nWindowWidth	= rWindow.right - rWindow.left;
	int nWindowHeight	= rWindow.bottom - rWindow.top;

	m_appState.hWnd = CreateWindowEx(WS_EX_APPWINDOW,		//	Extended Style flags.
		"DirectX_Spike",										//	Window Class Name.
		m_appState.szTitle,									//	Title of the Window.
		dwWindowStyleFlags,										//	Window Style Flags.
		(GetSystemMetrics(SM_CXSCREEN)/2) - (nWindowWidth/2),	//	Window Start Point (x, y). 
		(GetSystemMetrics(SM_CYSCREEN)/2) - (nWindowHeight/2),	//	Does the math to center the window over the desktop.
		nWindowWidth,											//	Width of Window.
		nWindowHeight,											//	Height of Window.
		NULL,													//	Handle to parent window.
		NULL,													//	Handle to menu.
		hInstance,												//	Application Instance.
		NULL);													//	Creation parameters.

	if (!m_appState.hWnd)
		return -1;

	IDirect3DDevice9* __theDevice(NULL);
	InitD3D(hInstance, nWindowWidth, nWindowHeight, windowed, D3DDEVTYPE_HAL, m_appState.hWnd, &__theDevice);
	::ShowCursor(false);

	float timeDelta(0.0f);
	LARGE_INTEGER lCurrent;
	LARGE_INTEGER lLastCount;
	LARGE_INTEGER lCounterFreq;
	QueryPerformanceCounter(&lLastCount);
	QueryPerformanceFrequency(&lCounterFreq);

	srand((unsigned int )lLastCount.QuadPart);

	//////////////////////////////////////////
	//	Initialize Game here
	//////////////////////////////////////////
	if(Setup(__theDevice)==false)
	{
		::MessageBox(0, _T("Setup Failed"), _T("Error"), 0);
		return FALSE;
	}
	
	// Event Loop
	while (TRUE)
	{
		/*if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))*/
		for( int i = 0; i < MAX_WIN_MSGS_PER_FRAME && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE); i++ )
		{ 
			if( msg.message == WM_PAINT )
			{
				continue;
			}

			//	Test if this is a quit
			if (msg.message == WM_QUIT)
				break;

			//	Translate any accelerator keys
			TranslateMessage(&msg);

			//	Send the message to the window proc
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT)
				break;

		//////////////////////////////////
		//	Put Game Logic Here
		//////////////////////////////////
		QueryPerformanceCounter(&lCurrent);
		timeDelta = float(lCurrent.QuadPart - lLastCount.QuadPart) / float(lCounterFreq.QuadPart);
		Update(__theDevice, timeDelta);
		Display(timeDelta, __theDevice);	

		lLastCount = lCurrent;

		if(timeDelta < .016f)
		{
			DWORD sleepTime = (DWORD)floorf(1000*(.016f-timeDelta));
			Sleep(sleepTime);
		}
	}

	/////////////////////////////////////////
	//	Shutdown Game Here
	/////////////////////////////////////////
	__theDevice->Release();
	Cleanup();

	//	Unregister the window class
	UnregisterClass(m_appState.szWindowClass, hInstance);

	//	Return to Windows like this.
	return (int)(msg.wParam);
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	//PAINTSTRUCT ps;
	//HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		/*switch (wmId)
		{
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:*/
			return DefWindowProc(hWnd, message, wParam, lParam);
		//}
		break;
	case WM_PAINT:
	//	hdc = BeginPaint(hWnd, &ps);
	//	EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	//case WM_MOUSEWHEEL:
	//	{
	//		int wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
	//		s_inputMgr.MouseWheel(wheelDelta);
	//	}
	case WM_KEYDOWN:
		{
			// ignore key repeats
			if( (lParam & (1<<29)) != 0 )
				break;

			switch( wParam )
			{
				case VK_F1:
					{
						static bool toggle = true;
						s_renderEngine.SetWireframeMode(toggle);
						toggle = !toggle;
					}
					break;
				case VK_F2:
					{
						static int blurs = 0;
						blurs++;
						if(blurs > MAXBLURS)
							blurs = 0;

						s_renderEngine.SetBlur(blurs);
						
					}
					break;
				case VK_F3:
					Settings_ToggleBool(DEBUG_DRAW_SCENE_TREE);
					break;
				case VK_F4:
					Settings_ToggleBool(HUD_SHOW_LIGHT_DIR);
					Settings_ToggleBool(DEBUG_ROTATE_SUN);
					break;
				case VK_F5:
					Settings_ToggleBool(HUD_SHOW_HUD);
					break;
				case VK_F6:
					s_renderEngine.ToggleClipMethod();
					break;
				case VK_F7:
					Settings_ToggleBool(DEBUG_SHOW_CLIP_BOUNDS);
					break;
				case KB_W: // w key
				case KB_S: // s key
				case KB_A: // a key
				case KB_D: // d key
					{
					s_inputMgr.KeyDown( wParam );
					}
					break;
				default:
					//assert(false);
					/* ignore unhandled key press */
					break;

			}
			break;
		}
		case WM_KEYUP:
			switch( wParam )
			{
				case KB_W: // w key
				case KB_S: // s key
				case KB_A: // a key
				case KB_D: // d key
					s_inputMgr.KeyUp( wParam );
					break;
				default:
					/* ignore unhandled key press */
					break;
			}
			break;
		case WM_INPUT:
			{
				UINT dwSize = 40;
				static BYTE lpb[40];
				GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER));
				RAWINPUT* raw = (RAWINPUT*)lpb;
		    
				if (raw->header.dwType == RIM_TYPEMOUSE) 
				{
					// update movement
					s_inputMgr.MouseMoved(raw->data.mouse.lLastX, raw->data.mouse.lLastY);

					// update wheel
					if( raw->data.mouse.usButtonFlags == RI_MOUSE_WHEEL )
					{
						s_inputMgr.MouseWheel((short)raw->data.mouse.usButtonData);
					}
				}
								
				MSG msg;
				if(PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE) && msg.message == WM_INPUT)
				{
					PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}

				break;
			}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void InitD3D(HINSTANCE hInstance, 
			 int width, int height,
			 bool windowedMode,
			 D3DDEVTYPE deviceType,
			 HWND hwnd,
			 IDirect3DDevice9** device) // out
{
	// get the pointer to directx 
	*device = NULL;
	IDirect3D9* _d3d9;
	_d3d9 = Direct3DCreate9(D3D_SDK_VERSION);

	// specify the parameters for the device object
	D3DPRESENT_PARAMETERS d3dpp;
	::ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.BackBufferWidth				= width;
	d3dpp.BackBufferHeight				= height;
	d3dpp.BackBufferFormat				= D3DFMT_A8R8G8B8;
	d3dpp.BackBufferCount				= 1;
	d3dpp.MultiSampleType				= D3DMULTISAMPLE_NONE; 
	d3dpp.MultiSampleQuality			= 0;
	d3dpp.SwapEffect					= D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow					= hwnd;
	d3dpp.Windowed						= windowedMode;
	d3dpp.EnableAutoDepthStencil		= true;
	d3dpp.AutoDepthStencilFormat		= D3DFMT_D24S8;
	d3dpp.Flags							= 0;
	d3dpp.FullScreen_RefreshRateInHz    = windowedMode ? D3DPRESENT_RATE_DEFAULT : 60;
	d3dpp.PresentationInterval			= D3DPRESENT_INTERVAL_IMMEDIATE;

	// check for hardware vertex processing
	D3DCAPS9 caps;
	_d3d9->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps);
	int TLMode = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	if(caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
	{
		TLMode = D3DCREATE_HARDWARE_VERTEXPROCESSING;
	}

	// create the device
	HRESULT hr = _d3d9->CreateDevice(D3DADAPTER_DEFAULT,
				deviceType, 
				hwnd,
				TLMode,
				&d3dpp,
				device);

	if(FAILED(hr))
	{
		::MessageBox(0, _T("D3DCreateDevice Failed"), _T("D3D Init Error"), 0);
		return;
	}

}

bool Setup(LPDIRECT3DDEVICE9 device)
{
	Settings_Init();

	// register the mouse for high res input
    RAWINPUTDEVICE Rid[1];
    Rid[0].usUsagePage = (USHORT) 0x01; 
    Rid[0].usUsage = (USHORT) 0x02; 
    Rid[0].dwFlags = RIDEV_NOLEGACY; //RIDEV_INPUTSINK;   
    Rid[0].hwndTarget = m_appState.hWnd;
    RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]));

	// setup the renderer
	s_renderEngine.SetDevice(device, WIDTH, HEIGHT);
	s_gameState.Init( &s_renderEngine );
	s_inputMgr.SetGameState(&s_gameState);

	s_hud.SetDevice(device);
	s_renderEngine.SetHUD(&s_hud);

	return true;
};

void Cleanup()
{
	s_gameState.ShutDown();
}

void Display(float elapsedmills, LPDIRECT3DDEVICE9 device)
{
	assert(device);
	s_renderEngine.RenderScene();
	device->Present(0, 0, 0, 0);
}

void Update( LPDIRECT3DDEVICE9 device, float elapsedmills )
{
	CalculateFPS(elapsedmills);
	D3DXVECTOR2 mousePos = s_inputMgr.GetMousePos();
	s_hud.SetCurrentMousePos(mousePos.x, mousePos.y);
	s_hud.SetCurrentFPS(m_appState.frameCount);
	CPlayer *player = s_gameState.GetPlayer();
	if( player )
	{
		s_hud.SetCurrentPlayerPos(player->GetPosition3D());
	}

	s_hud.SetCurrentLightDir(s_renderEngine.GetLightDirection());

	s_inputMgr.Update();
	s_renderEngine.Update( device, elapsedmills);
	s_gameState.Update( device, elapsedmills);
}

void CalculateFPS(float timeDelta)
{
	static int numFrames = 0;
	static float elapsedTime = 0.0f;
	elapsedTime += timeDelta;
	numFrames++;
	if(elapsedTime > 1.0f)
	{
		m_appState.frameCount = float(numFrames) / elapsedTime;

		numFrames = 0;
		elapsedTime = 0.0f;
	}

}