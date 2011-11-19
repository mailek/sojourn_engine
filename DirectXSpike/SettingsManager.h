#pragma once

#include "singleton.h"
#include "settings.h"

typedef struct
{
	SettingValue	value;
	ESettingId		key;
} SettingValueLookupTable[SETTINGS_CNT];

class CSettingsManager : public SingleThreadSingleton<CSettingsManager>
{
	friend class SingleThreadSingleton<CSettingsManager>;
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
