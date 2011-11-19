#include "StdAfx.h"
#include "SettingsManager.h"

CSettingsManager::CSettingsManager(void)
{
}

CSettingsManager::~CSettingsManager(void)
{
}

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

void CSettingsManager::GetSettingValue(ESettingId id, SettingValue** out)
{
	assert(id == m_settings[id].key);
	*out = &m_settings[id].value;
}

void CSettingsManager::SetSettingValue(ESettingId id, SettingValue* in)
{
	assert(id == m_settings[id].key);
	m_settings[id].value = *in;
}