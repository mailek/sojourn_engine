#pragma once
/********************************************************************
	created:	2012/04/16
	filename: 	GameEvents.h
	author:		Matthew Alford
	
	purpose:	
*********************************************************************/
#include "MathUtil.h"

class CTerrainContainer;

namespace GameEvents 
{

interface IEventHandler 
{
	virtual bool HandleEvent( UINT eventId, void* data, UINT data_sz )=0;
};

//////////////////////////////////////
// Events
//////////////////////////////////////
typedef enum _EEventIds
{
	EVT_UPDATE		 = 1, 
	EVT_RENDER,			  
	EVT_INIT,       
	EVT_DESTROY,    
	EVT_GETFACINGVEC,
	EVT_SETFACINGVEC,
	EVT_GETPOSITIONVEC,
	EVT_SETPOSITIONVEC,
	EVT_SETAVATAR,
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
	EVT_TERRAIN_GRIDCHANGED,
	EVT_CNT
} EEventIds;

//////////////////////////////////////
// Event Arg Types
//////////////////////////////////////

// EVT_MOVECMD
typedef enum _EMovementStateType
{
	MOVE_FORWARD, 
	MOVE_BACKWARD, 
	MOVE_LEFT, 
	MOVE_RIGHT
	/* note: order matters */
} EMovementStateType;

typedef struct _MovementCommandType
{
	EMovementStateType	state;
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

// EVT_SETAVATAR
typedef IEventHandler* SetAvatarEventArgType;

// EVT_GETAVATAR
typedef IEventHandler** GetAvatarEventArgType;

//EVT_TERRAIN_GRIDCHANGED
typedef CTerrainContainer* TerrainGridChangedArgType;

} /* end namespace GameEvents */