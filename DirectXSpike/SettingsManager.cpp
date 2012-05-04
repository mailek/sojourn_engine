/********************************************************************
	created:	2012/04/16
	filename: 	SettingsManager.cpp
	author:		Matthew Alford
	
	purpose:	
*********************************************************************/
#include "StdAfx.h"
#include "SettingsManager.h"

//////////////////////////////////////////////////////////////////////////
// Setup Functions
//////////////////////////////////////////////////////////////////////////

CSettingsManager::CSettingsManager(void)
{
}

CSettingsManager::~CSettingsManager(void)
{
}

/************************************************
*   Name:   CSettingsManager::Init
*   Desc:   
************************************************/
void CSettingsManager::Init()
{
	for(int i = 0; i < SETTINGS_CNT; i++)
	{
		SettingValue* setting = &m_settings[i].value;
		assert(i == SettingsValueInitTable[i].id);
		m_settings[i].key = (ESettingId)i;

		switch(SettingsValueInitTable[i].init)
		{
		case BOOL_TRUE:
			InitVariantBool(setting, true);
			break;
		case BOOL_FALSE:
			InitVariantBool(setting, false);
			break;
		default:
			assert(false);
			break;

		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Update Functions
//////////////////////////////////////////////////////////////////////////

/************************************************
*   Name:   CSettingsManager::GetSettingValue
*   Desc:   
************************************************/
void CSettingsManager::GetSettingValue(ESettingId id, SettingValue** out)
{
	assert(id == m_settings[id].key);
	*out = &m_settings[id].value;
}

/************************************************
*   Name:   CSettingsManager::SetSettingValue
*   Desc:   
************************************************/
void CSettingsManager::SetSettingValue(ESettingId id, SettingValue* in)
{
	assert(id == m_settings[id].key);
	m_settings[id].value = *in;
}