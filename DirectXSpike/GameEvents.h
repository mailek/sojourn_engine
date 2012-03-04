#pragma once

#include "MathUtil.h"

interface IEventHandler 
{
	virtual bool HandleEvent( UINT eventId, void* data, UINT data_sz )=0;
};

//////////////////////////////////////
// Events
//////////////////////////////////////
typedef enum {
	EVT_UPDATE		 = 1, 
	EVT_RENDER,			  
	EVT_INIT,       
	EVT_DESTROY,    
	EVT_GETFACINGVEC,
	EVT_SETFACINGVEC,
	EVT_GETPOSITIONVEC,
	EVT_SETPOSITIONVEC,
	EVT_GETAVATAR,
	EVT_DEBUGCAMERA,
	EVT_CONTROL_GAIN_FOCUS,
	EVT_CONTROL_LOST_FOCUS,
	EVT_KEYUP,      
	EVT_KEYDOWN,    
	EVT_MOUSE_MOVED,
	EVT_MOUSE_WHEEL,
	EVT_MOVECMD,
	EVT_GETTEXCONTEXT,
	EVT_ATTACH_CAMERA,
	EVT_CNT
};

//////////////////////////////////////
// Event Arg Types
//////////////////////////////////////

// EVT_MOVECMD
typedef enum 
{
	MOVE_FORWARD, 
	MOVE_BACKWARD, 
	MOVE_LEFT, 
	MOVE_RIGHT
	/* note: order matters */
} MovementStateType;

typedef struct
{
	MovementStateType	state;
	bool				clearFlag;
} MovementCommandType;

// EVT_UPDATE
typedef float UpdateEventArgType;

// EVT_GETFACINGVEC
typedef Vector_3 GetFacingEventArgType;

// EVT_SETFACINGVEC
typedef Vector_3 SetFacingEventArgType;

// EVT_GETPOSITIONVEC
typedef Vector_3 GetPosEventArgType;

// EVT_SETPOSITIONVEC
typedef Vector_3 SetPosEventArgType;