#pragma once
//////////////////////////////////////////////////////////////////////////
// StdAfx.h - 2011 Matthew Alford
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////
// Includes
//////////////////////////////////////
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <vector>
#include <assert.h>
#include <map>
#include <list>
#include <math.h>

//////////////////////////////////////
// Constants
//////////////////////////////////////
#define NULL			0
#define BITS_PER_BYTE	(16)
#define DRAW_DIST		(2500)
#define SCREEN_RES_H	(1280)
#define SCREEN_RES_V	(1024)

//////////////////////////////////////
// Forward Declarations
//////////////////////////////////////
extern inline void COM_SAFERELEASE(IUnknown* ptr);

//////////////////////////////////////
// Macros
//////////////////////////////////////

/* memory */
#define PTR_SAFEDELETE(p) \
	{ delete p; p = NULL;	}

/* utility */
#define cnt_of_array(type) \
	( sizeof(type) / sizeof(type[0]) )

/* verify hresult */
#define HR(com) \
	if( FAILED(com) ) \
		{assert(false);}

//////////////////////////////////////
// Events
//////////////////////////////////////
typedef enum {
	EVT_UPDATE		 = 1,
	EVT_RENDER,     
	EVT_INIT,       
	EVT_DESTROY,    
	EVT_GETPLAYER,  
	EVT_KEYUP,      
	EVT_KEYDOWN,    
	EVT_MOUSE_MOVED,
	EVT_MOUSE_WHEEL,

	EVT_CNT
};

//////////////////////////////////////
// Types
//////////////////////////////////////
#define ColorRGBA32 D3DCOLORVALUE