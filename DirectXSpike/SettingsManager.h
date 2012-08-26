#pragma once
/********************************************************************
	created:	2012/04/16
	filename: 	SettingsManager.h
	author:		Matthew Alford
	
	purpose:	
*********************************************************************/
#include "singleton.h"
#include "settings.h"

typedef struct _SettingValueLookupTable
{
	SettingValue	value;
	ESettingId		key;
} SettingValueLookupTable[SETTINGS_CNT];

class CSettingsManager : public SingleThreadSingleton<CSettingsManager, CSettingsManager>
{
	friend class SingleThreadSingleton<CSettingsManager, CSettingsManager>;
private:
	CSettingsManager(void);
	~CSettingsManager(void);

public:
	void Init();
	void GetSettingValue(ESettingId id, SettingValue** out);
	void SetSettingValue(ESettingId id, SettingValue* in);

private:
	SettingValueLookupTable		m_settings;
};

inline bool Settings_GetBool(ESettingId id)								
{																	
	SettingValue* setting;											
	CSettingsManager::GetInstance()->GetSettingValue(id, &setting);	
	return setting->value.bool_val;								
}																	