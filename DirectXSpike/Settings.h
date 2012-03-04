#pragma once

#include "variantutil.h"

typedef Variant SettingValue;

typedef enum
{
	INVALID_VAL = 0,
	BOOL_TRUE,
	BOOL_FALSE
} ESettingInitValue;

typedef enum
{
	/* ==== Debug Flags ==== */
	DEBUG_DRAW_SCENE_TREE = 0,	// render or hide quad tree lines
	DEBUG_ROTATE_SUN,			// cause sun light source to rotate slowly
	DEBUG_SHOW_CLIP_BOUNDS,		// draw camera clip bound spheres
	DEBUG_CAMERA_DETACH,		// free-fly camera
	/* ==== HUD Info	==== */
	HUD_SHOW_HUD,				// HUD visibility
	HUD_SHOW_LIGHT_DIR,			// sun light source direction

	SETTINGS_CNT
} ESettingId;

/* must be in same order as ESettingId */
const struct
{
	ESettingId			id;
	ESettingInitValue	init;

} SettingsValueInitTable[SETTINGS_CNT] =
{
	{DEBUG_DRAW_SCENE_TREE, BOOL_FALSE},
	{DEBUG_ROTATE_SUN,		BOOL_FALSE},
	{DEBUG_SHOW_CLIP_BOUNDS,BOOL_FALSE},
	{DEBUG_CAMERA_DETACH,	BOOL_FALSE},
	{HUD_SHOW_HUD,			BOOL_FALSE},
	{HUD_SHOW_LIGHT_DIR,	BOOL_FALSE}
};

/* Helper Functions */
#define Settings_Init()												\
	CSettingsManager::GetInstance()->Init();						

#define Settings_GetBool(id, boolean)								\
{																	\
	SettingValue* setting;											\
	CSettingsManager::GetInstance()->GetSettingValue(id, &setting);	\
	boolean = setting->value.bool_val;								\
}																	

#define Settings_ToggleBool(id)										\
{																	\
	SettingValue s, *p;												\
																	\
	CSettingsManager::GetInstance()->GetSettingValue(id, &p);		\
	InitVariantBool(&s, !p->value.bool_val);						\
	CSettingsManager::GetInstance()->SetSettingValue(id, &s);		\
}																	

#include "settingsmanager.h"