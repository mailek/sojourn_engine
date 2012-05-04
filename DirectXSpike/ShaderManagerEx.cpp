/********************************************************************
	created:	2012/04/16
	filename: 	ShaderManagerEx.cpp
	author:		Matthew Alford
	
	purpose:	
*********************************************************************/
#include "StdAfx.h"
#include "ShaderManagerEx.h"

//////////////////////////////////////////////////////////////////////////
// Setup Functions
//////////////////////////////////////////////////////////////////////////

CShaderManagerEx::CShaderManagerEx(void) : m_currentEffect(NULL)
{
}

CShaderManagerEx::~CShaderManagerEx(void)
{
	COM_SAFERELEASE(m_currentEffect);
}

/************************************************
*   Name:   CShaderManagerEx::Setup
*   Desc:   
************************************************/
void CShaderManagerEx::Setup(void)
{
	/* call the base class */
	CShaderManager::Setup();

	/* create the materials (effects) */
	LoadShader( "SkyDome.fx",		EFFECT_SKYDOME );
	LoadShader( "GaussBlur.fx",		EFFECT_GAUSSBLUR );
	LoadShader( "ScreenQuad.fx",	EFFECT_SCREENQUAD );
	LoadShader( "DebugDraw.fx",	    EFFECT_DEBUGDRAW );
	LoadShader( "FlatShade.fx",		EFFECT_LIGHTTEX );

	/* DIRECTIONAL LIGHTS */
	D3DXCOLOR lightColor = D3DXCOLOR(1.f, 1.0f, 1.0f, 1.0f);
	::ZeroMemory(&m_DirectionalLight, sizeof(m_DirectionalLight));
	m_DirectionalLight.Type			= D3DLIGHT_DIRECTIONAL;
	m_DirectionalLight.Ambient		= lightColor * 0.5f;
	m_DirectionalLight.Diffuse		= lightColor;
	m_DirectionalLight.Specular		= lightColor * 0.3f;
	m_DirectionalLight.Direction	= D3DXVECTOR3(0.0f, -1.0f, 0.0f);//D3DXVECTOR3(-0.7f, -1.0f, -0.7f);
}

/************************************************
*   Name:   CShaderManagerEx::LoadShader
*   Desc:   
************************************************/
void CShaderManagerEx::LoadShader( const char* filename, int index )
{
	char			file[200];
	ID3DXBuffer*	errors = NULL;

	strcpy_s( file, "Shaders//" );
	strcat_s( file, filename );

	D3DXCreateEffectFromFile( 
		m_device, 
		file, 
		NULL, 
		NULL, 
		0/*no flags*/, 
		NULL, 
		&m_effectSet[index], 
		&errors );

	if( errors )
	{
		::MessageBox(0, (char*)errors->GetBufferPointer(), 0, 0 );
		COM_SAFERELEASE( errors );
		return;
	}

}

//////////////////////////////////////////////////////////////////////////
// Update Functions
//////////////////////////////////////////////////////////////////////////

/************************************************
*   Name:   CShaderManagerEx::SetWorldTransformEx
*   Desc:   
************************************************/
void CShaderManagerEx::SetWorldTransformEx(D3DXMATRIX worldTransform)
{
	D3DXHANDLE h=0;
	h = m_currentEffect->GetParameterByName( NULL, "matWorld" );
	HR(m_currentEffect->SetMatrix(h, &worldTransform));
}

/************************************************
*   Name:   CShaderManagerEx::SetViewProjectionEx
*   Desc:   
************************************************/
void CShaderManagerEx::SetViewProjectionEx(D3DXMATRIX viewTransform, D3DXMATRIX projectionTransform)
{
	D3DXHANDLE h=0;
	D3DXMATRIX viewProj = viewTransform * projectionTransform;

	h = m_currentEffect->GetParameterByName( NULL, "matViewProjection" );
	HR(m_currentEffect->SetMatrix(h, &viewProj));

	h = m_currentEffect->GetParameterByName( NULL, "matView" );
	if( h )
	{
		HR(m_currentEffect->SetMatrix(h, &viewTransform));
	}

}

/************************************************
*   Name:   CShaderManagerEx::BeginEffect
*   Desc:   
************************************************/
int CShaderManagerEx::BeginEffect()
{
	UINT numPasses;
	m_device->SetVertexShader(NULL);
	m_device->SetPixelShader(NULL);
	HR(m_currentEffect->Begin(&numPasses, 0));

	return (int)numPasses;
}

/************************************************
*   Name:   CShaderManagerEx::Pass
*   Desc:   
************************************************/
void CShaderManagerEx::Pass(UINT pass)
{
	HR(m_currentEffect->BeginPass(pass));
}

/************************************************
*   Name:   CShaderManagerEx::SetDefaultTechnique
*   Desc:   
************************************************/
void CShaderManagerEx::SetDefaultTechnique()
{
	D3DXHANDLE hTechnique=0;
	HR(m_currentEffect->FindNextValidTechnique(NULL, &hTechnique));
	HR(m_currentEffect->SetTechnique(hTechnique));
}

/************************************************
*   Name:   CShaderManagerEx::SetTechnique
*   Desc:   
************************************************/
void CShaderManagerEx::SetTechnique(LPCSTR name)
{
	D3DXHANDLE hTechnique = m_currentEffect->GetTechniqueByName(name);
	HR(m_currentEffect->SetTechnique(hTechnique));
}

/************************************************
*   Name:   CShaderManagerEx::FinishEffect
*   Desc:   
************************************************/
void CShaderManagerEx::FinishEffect()
{
	HR(m_currentEffect->End());
}

/************************************************
*   Name:   CShaderManagerEx::FinishPass
*   Desc:   
************************************************/
void CShaderManagerEx::FinishPass()
{
	HR(m_currentEffect->EndPass());
}

/************************************************
*   Name:   CShaderManagerEx::SetEffect
*   Desc:   
************************************************/
void CShaderManagerEx::SetEffect(EEffectID effectId)
{
	assert(effectId < EFFECT_CNT && effectId >= 0);

	if( m_currentEffect != m_effectSet[effectId] )
	{
		m_currentEffect = m_effectSet[effectId];
		ReloadEffect(effectId);
	}
}

/************************************************
*   Name:   CShaderManagerEx::ReloadEffect
*   Desc:   
************************************************/
void CShaderManagerEx::ReloadEffect(EEffectID effectId)
{
	switch(effectId)
	{
	case EFFECT_LIGHTTEX:
	case EFFECT_SKYDOME:
		break;
	case EFFECT_GAUSSBLUR:
	case EFFECT_SCREENQUAD:
		{
			ShaderVariant v;
			v.type = SHADER_VAR_FLOAT;
			v._float = 1.0f/m_iViewportWidth;
			SetEffectConstant( "fInvViewportWidth", v );
			v._float = 1.0f/m_iViewportHeight;
			SetEffectConstant( "fInvViewportHeight", v );
			break;
		}
	case EFFECT_DEBUGDRAW:
		break;
	default:
		assert(false);
		break;
	}
}

/************************************************
*   Name:   CShaderManagerEx::SetEffectConstant
*   Desc:   
************************************************/
bool CShaderManagerEx::SetEffectConstant( LPCSTR name, ShaderVariant &variant )
{
	D3DXHANDLE constHnd = m_currentEffect->GetParameterByName( NULL, name );
	if(constHnd)
	{
		switch(variant.type)
		{
		case SHADER_VAR_MATRIX:
			HR(m_currentEffect->SetFloatArray( constHnd, variant._matrix, 16 ));
			break;
		case SHADER_VAR_FLOAT4:
			{
			HR(m_currentEffect->SetFloatArray( constHnd, variant._float4, 4 ));
			break;
			}
		case SHADER_VAR_FLOAT:
			HR(m_currentEffect->SetFloat( constHnd, variant._float ));
			break;
		default:
			assert(false);
			break;
		}
	} 
	else 
	{
		assert(false);
		return false;
	}

	return true;
}

/************************************************
*   Name:   CShaderManagerEx::SetTexture
*   Desc:   
************************************************/
void CShaderManagerEx::SetTexture(LPCSTR name, LPDIRECT3DTEXTURE9 texture)
{
	D3DXHANDLE hnd = m_currentEffect->GetParameterByName(0, name);
	HR(m_currentEffect->SetTexture(hnd, texture));
}

/************************************************
*   Name:   CShaderManagerEx::SetDrawColorEx
*   Desc:   
************************************************/
void CShaderManagerEx::SetDrawColorEx(D3DXCOLOR color)
{
	ShaderVariant v;

	v.type = SHADER_VAR_FLOAT4;
	v._float4[0] = color.r;
	v._float4[1] = color.g;
	v._float4[2] = color.b;
	v._float4[3] = color.a;

	SetEffectConstant( "drawColor", v );
}

/************************************************
*   Name:   CShaderManagerEx::CommitEffectParams
*   Desc:   
************************************************/
void CShaderManagerEx::CommitEffectParams()
{
	HR(m_currentEffect->CommitChanges());
}

/************************************************
*   Name:   CShaderManagerEx::SetMaterialEx
*   Desc:   
************************************************/
void CShaderManagerEx::SetMaterialEx( const D3DMATERIAL9 &mat )
{
	ShaderVariant v;

	v.type = SHADER_VAR_FLOAT4;
	memcpy( v._float4, &mat.Diffuse, sizeof(v._float4) );
	SetEffectConstant( "diffuseMaterial", v );

	memcpy( v._float4, &mat.Ambient, sizeof(v._float4) );
	SetEffectConstant( "ambientMaterial", v );
}

/************************************************
*   Name:   CShaderManagerEx::SetLightDirection
*   Desc:   
************************************************/
void CShaderManagerEx::SetLightDirection(D3DXVECTOR3& lightDir)
{
	ShaderVariant v;

	v.type = SHADER_VAR_FLOAT4;

	v._float4[0] = lightDir.x;
	v._float4[1] = lightDir.y;
	v._float4[2] = lightDir.z;
	v._float4[3] = 1.0f;

	SetEffectConstant( "directionToLight", v );
}