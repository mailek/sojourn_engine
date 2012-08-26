#pragma once
//////////////////////////////////////////////////////////////////////////
// StdAfx.h - 2011 Matthew Alford
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////
// Includes
//////////////////////////////////////
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h>

#include <vector>
#include <map>
#include <list>
#include <math.h>

#include "DirectXUtil.h"
#include "AssertUtil.h"
#include "MathUtil.h"

//////////////////////////////////////
// Constants
//////////////////////////////////////
#define NULL				0
#define BITS_PER_BYTE		(8)
#define DRAW_DIST			(2500)
#define SCREEN_RES_H		(1280)
#define SCREEN_RES_V		(1024)
#define MAX_FILENAME_LEN	(200)

#define ENGINE_FOLDER		"./"
#define SHADERS_FOLDER		ENGINE_FOLDER"Shaders/"
#define RESOURCE_FOLDER		ENGINE_FOLDER"Resources/"
#define TEST_ROOT_FOLDER	ENGINE_FOLDER"../DirectXSpike/Testing/"

#define BREAK_ON_FAILURES

//////////////////////////////////////
// Forward Declarations
//////////////////////////////////////
extern inline void COM_SAFERELEASE(IUnknown* ptr);
extern inline const char* _sprintf(char* dest, const char* format, ...);
extern inline void CONSOLE(const char* format, ...);
extern inline void CONSOLE(char* msg);

//////////////////////////////////////
// Macros
//////////////////////////////////////

/* memory free */
#define ptr_safedelete(p) \
	{ delete p; p = NULL;	}

/* array size */
#define cnt_of_array(type) \
	( sizeof(type) / sizeof(type[0]) )

/* static cast */
#define __sc(cast_type, val) static_cast<cast_type>(val)

//////////////////////////////////////
// Types
//////////////////////////////////////

typedef Vector_3 WorldPosition;