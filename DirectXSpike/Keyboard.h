#pragma once
//////////////////////////////////////////////////////////////////////////
// DirectXSpike.cpp : Defines the entry point for the application.
// 2011 Matthew Alford
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////
// Global Constants
//////////////////////////////////////
enum KB_Key_Definitions {
	KB_NUM_OFFSET	= 0x30,
	KB_0		= 0x30,
	KB_1,
	KB_2,
	KB_3,
	KB_4,
	KB_5,
	KB_6,
	KB_7,
	KB_8,
	KB_9,
	KB_ALPHA_OFFSET = 0x41,
	KB_A		= 0x41,
	KB_B,
	KB_C,
	KB_D,
	KB_E,
	KB_F,
	KB_G,
	KB_H,
	KB_I,
	KB_J,
	KB_K,
	KB_L,
	KB_M,
	KB_N,
	KB_O,
	KB_P,
	KB_Q,
	KB_R,
	KB_S,
	KB_T,
	KB_U,
	KB_V,
	KB_W,
	KB_X,
	KB_Y,
	KB_Z,
	KB_CNT = 36
};

enum Virtual_Key_Definitions {
	VGK_INVALID = 0,
	VGK_DIRUP,
	VGK_DIRDOWN,
	VGK_DIRLEFT,
	VGK_DIRRIGHT,
	VGK_CNT
};

typedef struct {
	UINT kb;
	UINT vk;
} KEYBINDING;

static const KEYBINDING s_keybindings[KB_CNT] = {
	{KB_0, VGK_INVALID},
	{KB_1, VGK_INVALID},
	{KB_2, VGK_INVALID},
	{KB_3, VGK_INVALID},
	{KB_4, VGK_INVALID},
	{KB_5, VGK_INVALID},
	{KB_6, VGK_INVALID},
	{KB_7, VGK_INVALID},
	{KB_8, VGK_INVALID},
	{KB_9, VGK_INVALID},
	{KB_A, VGK_DIRLEFT},
	{KB_B, VGK_INVALID},
	{KB_C, VGK_INVALID},
	{KB_D, VGK_DIRRIGHT},
	{KB_E, VGK_INVALID},
	{KB_F, VGK_INVALID},
	{KB_G, VGK_INVALID},
	{KB_H, VGK_INVALID},
	{KB_I, VGK_INVALID},
	{KB_J, VGK_INVALID},
	{KB_K, VGK_INVALID},
	{KB_L, VGK_INVALID},
	{KB_M, VGK_INVALID},
	{KB_N, VGK_INVALID},
	{KB_O, VGK_INVALID},
	{KB_P, VGK_INVALID},
	{KB_Q, VGK_INVALID},
	{KB_R, VGK_INVALID},
	{KB_S, VGK_DIRDOWN},
	{KB_T, VGK_INVALID},
	{KB_U, VGK_INVALID},
	{KB_V, VGK_INVALID},
	{KB_W, VGK_DIRUP},
	{KB_X, VGK_INVALID},
	{KB_Y, VGK_INVALID},
	{KB_Z, VGK_INVALID}
};

static inline UINT TranslateKey( UINT keyboard )
{
	// Translate to virtual key through key bindings
	UINT idx = 0;
	if( keyboard < KB_A )
	{
		idx = keyboard - KB_NUM_OFFSET;
	}
	else
	{
		idx = keyboard - KB_ALPHA_OFFSET + 10;
	}

	assert( keyboard == s_keybindings[idx].kb );
	return s_keybindings[idx].vk;
}