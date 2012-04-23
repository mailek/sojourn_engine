#pragma once
/********************************************************************
	created:	2012/04/16
	filename: 	ShaderManagerEx.h
	author:		Matthew Alford
	
	purpose:	
*********************************************************************/
#include "shadermanager.h"

typedef enum _EEffectID 
{
	EFFECT_LIGHTTEX,
	EFFECT_SKYDOME,
	EFFECT_GAUSSBLUR,
	EFFECT_SCREENQUAD,
	EFFECT_DEBUGDRAW,
	EFFECT_CNT,
	EFFECT_INVALID = -1
} EEffectID;

class CShaderManagerEx :
	public CShaderManager
{
public:
	CShaderManagerEx(void);
	~CShaderManagerEx(void);

	virtual void Setup();

	void SetWorldTransformEx(D3DXMATRIX worldTransform);
	void SetViewProjectionEx(D3DXMATRIX viewTransform, D3DXMATRIX projectionTransform);
	void SetDrawColorEx(D3DXCOLOR color);
	void SetMaterialEx( const D3DMATERIAL9 &mat );
	int BeginEffect(); // returned num of passes
	void Pass(UINT pass);
	void SetTechnique(LPCSTR name);
	void SetDefaultTechnique();
	void CommitEffectParams();
	void FinishEffect();
	void FinishPass();
	void SetEffect(EEffectID effectId);
	void ReloadEffect(EEffectID effectId);
	bool SetEffectConstant( LPCSTR name, ShaderVariant &variant );
	void SetTexture(LPCSTR name, LPDIRECT3DTEXTURE9 texture);
	void SetLightDirection(D3DXVECTOR3& lightDir);

private:
	LPD3DXEFFECT	m_currentEffect;
	LPD3DXEFFECT	m_effectSet[EFFECT_CNT];

	void LoadShader( const char* filename, int index );

};
