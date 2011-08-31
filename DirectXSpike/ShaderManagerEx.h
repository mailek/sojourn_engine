#pragma once
#include "shadermanager.h"

enum eEffectID {
		EFFECT_SKYDOME,
		EFFECT_GAUSSBLUR,
		EFFECT_CNT,
		EFFECT_INVALID = -1
	};

class CShaderManagerEx :
	public CShaderManager
{
public:
	CShaderManagerEx(void);
	~CShaderManagerEx(void);

	virtual void Setup();

	void SetWorldTransformEx(D3DXMATRIX worldTransform);
	void SetViewProjectionEx(D3DXMATRIX viewProjTransform);
	int BeginEffect(); // returned num of passes
	void Pass(UINT pass);
	void SetTechnique(LPCSTR name);
	void SetDefaultTechnique();
	void FinishEffect();
	void FinishPass();
	void SetEffect(eEffectID effectId);
	void ReloadEffect(eEffectID effectId);
	bool SetEffectConstant( LPCSTR name, ShaderVariant &variant );
	void SetTexture(LPCSTR name, LPDIRECT3DTEXTURE9 texture);

private:
	LPD3DXEFFECT	m_currentEffect;
	LPD3DXEFFECT	m_effectSet[EFFECT_CNT];

};
