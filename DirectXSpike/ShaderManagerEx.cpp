#include "StdAfx.h"
#include "ShaderManagerEx.h"

CShaderManagerEx::CShaderManagerEx(void) : m_currentEffect(NULL)
{
}

CShaderManagerEx::~CShaderManagerEx(void)
{
	COM_SAFERELEASE(m_currentEffect);
}

void CShaderManagerEx::Setup(void)
{
	// call the base class
	CShaderManager::Setup();

	// create the materials (effects)
	LoadShader( "SkyDome.fx",		EFFECT_SKYDOME );
	LoadShader( "GaussBlur.fx",		EFFECT_GAUSSBLUR );
	LoadShader( "ScreenQuad.fx",	EFFECT_SCREENQUAD );
	LoadShader( "Primitives.fx",	EFFECT_PRIMITIVES );
	LoadShader( "FlatShade.fx",		EFFECT_LIGHTTEX );
}

void CShaderManagerEx::SetWorldTransformEx(D3DXMATRIX worldTransform)
{
	D3DXHANDLE h=0;
	h = m_currentEffect->GetParameterByName( NULL, "matWorld" );
	HR(m_currentEffect->SetMatrix(h, &worldTransform));
}

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

int CShaderManagerEx::BeginEffect()
{
	UINT numPasses;
	m_device->SetVertexShader(NULL);
	m_device->SetPixelShader(NULL);
	HR(m_currentEffect->Begin(&numPasses, 0));

	return (int)numPasses;
}

void CShaderManagerEx::Pass(UINT pass)
{
	HR(m_currentEffect->BeginPass(pass));
}

void CShaderManagerEx::SetDefaultTechnique()
{
	D3DXHANDLE hTechnique=0;
	HR(m_currentEffect->FindNextValidTechnique(NULL, &hTechnique));
	HR(m_currentEffect->SetTechnique(hTechnique));
}

void CShaderManagerEx::SetTechnique(LPCSTR name)
{
	D3DXHANDLE hTechnique = m_currentEffect->GetTechniqueByName(name);
	HR(m_currentEffect->SetTechnique(hTechnique));
}

void CShaderManagerEx::FinishEffect()
{
	HR(m_currentEffect->End());
}

void CShaderManagerEx::FinishPass()
{
	HR(m_currentEffect->EndPass());
}

void CShaderManagerEx::SetEffect(eEffectID effectId)
{
	assert(effectId < EFFECT_CNT && effectId >= 0);

	if( m_currentEffect != m_effectSet[effectId] )
	{
		m_currentEffect = m_effectSet[effectId];
		ReloadEffect(effectId);
	}
}

void CShaderManagerEx::ReloadEffect(eEffectID effectId)
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
	case EFFECT_PRIMITIVES:
		break;
	default:
		assert(false);
		break;
	}
}

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
	} else 
	{
		assert(false);
		return false;
	}

	return true;
}

void CShaderManagerEx::SetTexture(LPCSTR name, LPDIRECT3DTEXTURE9 texture)
{
	D3DXHANDLE hnd = m_currentEffect->GetParameterByName(0, name);
	HR(m_currentEffect->SetTexture(hnd, texture));
}

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

void CShaderManagerEx::CommitEffectParams()
{
	HR(m_currentEffect->CommitChanges());
}

void CShaderManagerEx::SetMaterialEx( const D3DMATERIAL9 &mat )
{
	assert(false); // TODO: add support for setting materials on fx

	/*ShaderVariant* v = &m_vecPasses[pass].params.matDiffuse;
	v->type = SHADER_VAR_FLOAT4;
	memcpy( v->_float4, &mat.Diffuse, sizeof(v->_float4) );
	SetShaderConstant( m_vecPasses[pass].pVSConstTable, "diffuseMaterial", *v );
	
	v = &m_vecPasses[pass].params.matAmbient;
	v->type = SHADER_VAR_FLOAT4;
	memcpy( v->_float4, &mat.Ambient, sizeof(v->_float4) );
	SetShaderConstant( m_vecPasses[pass].pVSConstTable, "ambientMaterial", *v );*/
}