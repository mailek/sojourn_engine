/********************************************************************
	created:	2012/04/16
	filename: 	ShaderManager.cpp
	author:		Matthew Alford
	
	purpose:	
*********************************************************************/
#include "StdAfx.h"
#include "ShaderManager.h"
#include "Settings.h"

const float LightXRotationRateRadianSec = 1.0f;

//////////////////////////////////////////////////////////////////////////
// Setup Functions
//////////////////////////////////////////////////////////////////////////

CShaderManager::CShaderManager(void) : m_iViewportWidth(0),
										m_iViewportHeight(0),
										m_device(NULL),
										m_currentPS(PASS_INVALID),
										m_currentVS(PASS_INVALID)
{
}

CShaderManager::~CShaderManager(void)
{
	for(std::vector<ShaderPass>::iterator it = m_vecPasses.begin(), _it = m_vecPasses.end(); it != _it; ++it)
	{
		COM_SAFERELEASE(it->pPS);
		COM_SAFERELEASE(it->pVS);
		COM_SAFERELEASE(it->pPSConstTable);
		COM_SAFERELEASE(it->pVSConstTable);
	}
}

/************************************************
*   Name:   CShaderManager::SetDevice
*   Desc:   Set the DirectX device and initialize.
************************************************/
void CShaderManager::SetDevice(LPDIRECT3DDEVICE9 device) 
{
	m_device = device;
	Setup();
}

/************************************************
*   Name:   CShaderManager::Setup
*   Desc:   Load the individual shader programs.
************************************************/
void CShaderManager::Setup()
{
	////// FLAT SHADE PASS
	/*ShaderPass flatShadePass;
	::ZeroMemory(&flatShadePass, sizeof(flatShadePass));
	CreateVertexShader(flatShadePass, SHADERS_FOLDER"FlatShadeVS.fx");
	CreatePixelShader(flatShadePass, SHADERS_FOLDER"FlatShadePS.fx");
	m_vecPasses.push_back(flatShadePass);*/

	////// PLAIN RT PASS
	//ShaderPass plainRTPass;
	//::ZeroMemory(&plainRTPass, sizeof(plainRTPass));
	//CreateVertexShader(plainRTPass, SHADERS_FOLDER"PlainRTVS.fx");
	//CreatePixelShader(plainRTPass, "Shaders//BlurRTPS.fx");
	//CreatePixelShader(plainRTPass, SHADERS_FOLDER"PlainRTPS.fx");
	//m_vecPasses.push_back(plainRTPass);

	////// RENDERTARGET BLUR HORZ PASS
	//ShaderPass hblurRTPass;
	//::ZeroMemory(&hblurRTPass, sizeof(hblurRTPass));
	//hblurRTPass.pVS = m_vecPasses[PASS_RTPLAIN].pVS;
	//hblurRTPass.pVS->AddRef();
	//hblurRTPass.pVSConstTable = m_vecPasses[PASS_RTPLAIN].pVSConstTable;
	//hblurRTPass.pVSConstTable->AddRef();
	//CreatePixelShader(hblurRTPass, "Shaders//HorizontalGaussPS.fx");
	//m_vecPasses.push_back(hblurRTPass);

	//////// RENDERTARGET BLUR VERT PASS
	//ShaderPass vblurRTPass;
	//::ZeroMemory(&vblurRTPass, sizeof(vblurRTPass));
	//vblurRTPass.pVS = m_vecPasses[PASS_RTPLAIN].pVS;
	//vblurRTPass.pVS->AddRef();
	//vblurRTPass.pVSConstTable = m_vecPasses[PASS_RTPLAIN].pVSConstTable;
	//vblurRTPass.pVSConstTable->AddRef();
	//CreatePixelShader(vblurRTPass, "Shaders//VerticalGaussPS.fx");
	//m_vecPasses.push_back(vblurRTPass);

	////// PRIMITIVE SHADE PASS
	/*ShaderPass primShadePass;
	::ZeroMemory(&primShadePass, sizeof(primShadePass));
	CreateVertexShader(primShadePass, SHADERS_FOLDER"PrimitiveVS.fx");
	CreatePixelShader(primShadePass, SHADERS_FOLDER"PrimitivePS.fx");
	m_vecPasses.push_back(primShadePass);*/

	/////// PRIMITIVE COLORED PASS
	/*ShaderPass primColoredPass;
	::ZeroMemory(&primColoredPass, sizeof(primColoredPass));
	CreateVertexShader(primColoredPass, SHADERS_FOLDER"PrimitiveColoredVS.fx");
	primColoredPass.pPS = m_vecPasses[PASS_PRIMITIVE].pPS;
	primColoredPass.pPS->AddRef();
	primColoredPass.pPSConstTable = m_vecPasses[PASS_PRIMITIVE].pPSConstTable;
	primColoredPass.pPSConstTable->AddRef();
	m_vecPasses.push_back(primColoredPass);*/

	//// DIRECTIONAL LIGHTS
	//D3DXCOLOR lightColor = D3DXCOLOR(1.f, 1.0f, 1.0f, 1.0f);
	//::ZeroMemory(&m_DirectionalLight, sizeof(m_DirectionalLight));
	//m_DirectionalLight.Type			= D3DLIGHT_DIRECTIONAL;
	//m_DirectionalLight.Ambient		= lightColor * 0.5f;
	//m_DirectionalLight.Diffuse		= lightColor;
	//m_DirectionalLight.Specular		= lightColor * 0.3f;
	//m_DirectionalLight.Direction	= D3DXVECTOR3(0.0f, -1.0f, 0.0f);//D3DXVECTOR3(-0.7f, -1.0f, -0.7f);
}	
	
/************************************************
*   Name:   CShaderManager::CreatePixelShader
*   Desc:   Compile a pixel shader 2.0 program
*           specified by a given filename.
************************************************/
void CShaderManager::CreatePixelShader(ShaderPass &pass, LPCSTR psFileName)
{
	ID3DXBuffer* shader(NULL);
	ID3DXBuffer* errors(NULL);

	HRESULT hr = D3DXCompileShaderFromFile(psFileName, 0, 0, 
		"ps_main", "ps_2_0", D3DXSHADER_DEBUG, 
		&shader, &errors, &pass.pPSConstTable);

	if(errors || FAILED(hr))
	{
		std::string errorsString((char*)errors->GetBufferPointer());
		::MessageBox(0, errorsString.c_str(), "D3DXCompileShader Failed!", 0);
	}
	
	HR(m_device->CreatePixelShader((DWORD*)shader->GetBufferPointer(), &pass.pPS));
	pass.pPSConstTable->SetDefaults(m_device);

	COM_SAFERELEASE(shader);
	COM_SAFERELEASE(errors);
}

/************************************************
*   Name:   CShaderManager::CreateVertexShader
*   Desc:   Compile a vertex shader 2.0 program
*           specified by a given filename.
************************************************/
void CShaderManager::CreateVertexShader(ShaderPass &pass, LPCSTR vsFileName)
{
	ID3DXBuffer* shader(NULL);
	ID3DXBuffer* errors(NULL);

	HR(D3DXCompileShaderFromFile(vsFileName, 0, 0, 
		"vs_main", "vs_2_0", D3DXSHADER_DEBUG, 
		&shader, &errors, &pass.pVSConstTable));

	if(errors)
	{
		std::string errorsString((char*)errors->GetBufferPointer());
		::MessageBox(0, errorsString.c_str(), "D3DXCompileShader Failed!", 0);
	}
	
	HR(m_device->CreateVertexShader((DWORD*)shader->GetBufferPointer(), &pass.pVS));
	pass.pVSConstTable->SetDefaults(m_device);

	COM_SAFERELEASE(shader);
	COM_SAFERELEASE(errors);
}

//////////////////////////////////////////////////////////////////////////
// Update Functions
//////////////////////////////////////////////////////////////////////////

/************************************************
*   Name:   CShaderManager::Update
*   Desc:   Rotate the parallel light at a fixed
*           constant rate.
************************************************/
void CShaderManager::Update(float elapsed)
{
	if( Settings_GetBool(DEBUG_ROTATE_SUN) )
	{
		float rotationAmount = elapsed * LightXRotationRateRadianSec;
		D3DXVECTOR3 direction = m_DirectionalLight.Direction, outDir;
		D3DXMATRIX rotMatrix;
		D3DXVec3TransformCoord(&outDir, &direction, D3DXMatrixRotationX(&rotMatrix, rotationAmount) );
		m_DirectionalLight.Direction = outDir;
	}
}

/************************************************
*   Name:   CShaderManager::SetVertexShader
*   Desc:   Activate a vertex shader program, and
*           reload its constant table.
************************************************/
void CShaderManager::SetVertexShader(EPassId vtShaderId)
{
	m_device->SetVertexShader(GetVertexShaderByPass(vtShaderId));

	ReloadVertexShader(vtShaderId);

	/* must be set after reload */
	m_currentVS = vtShaderId;
}

/************************************************
*   Name:   CShaderManager::SetPixelShader
*   Desc:   Activate a pixel shader program, and
*           reload its constant table.
************************************************/
void CShaderManager::SetPixelShader(EPassId pxShaderId)
{
	m_device->SetPixelShader(GetPixelShaderByPass(pxShaderId));
	
	ReloadPixelShader(pxShaderId);

	/* must be set after reload */
	m_currentPS = pxShaderId;
}

/************************************************
*   Name:   CShaderManager::ReloadPixelShader
*   Desc:   Apply the latest shader constants
*           (uniforms) to the given pixel program.
************************************************/
void CShaderManager::ReloadPixelShader(EPassId passId)
{
	if(m_currentPS == passId)
		return;

	/*switch(passId)
	{*/
	//case PASS_DEFAULT:
	//	break;
	/*case PASS_RTPLAIN:
		m_device->SetRenderState(D3DRS_FOGENABLE, FALSE);
		break;*/
	//case PASS_RTBLURH:
	//	{
	//	SetInvViewport(PASS_RTPLAIN, (float)m_iViewportWidth, (float)m_iViewportHeight);
	//	break;
	//	}
	//case PASS_RTBLURV:
	//	{
	//	SetInvViewport(PASS_RTPLAIN, (float)m_iViewportWidth, (float)m_iViewportHeight);
	//	break;
	//	}
	//case PASS_PRIMITIVE:
	//case PASS_PRIM_COLORED:
	//	break;
	/*default:
		assert(false);
		break;
	}*/

}

/************************************************
*   Name:   CShaderManager::ReloadVertexShader
*   Desc:   Apply the latest shader constants
*           (uniforms) to the given vertex program.
************************************************/
void CShaderManager::ReloadVertexShader(EPassId passId)
{
	if(m_currentVS == passId)
		return;

	ShaderPass &sp = m_vecPasses[passId];
	sp.pVSConstTable->SetDefaults(m_device);

	/*switch(passId)
	{
	case PASS_DEFAULT:
		{*/

		// TODO: clean this up and move to shadermgrex

		// Flat Shade
		SetViewProjection( passId, sp.params.view, sp.params.projection );

		D3DXVECTOR4 toLight = D3DXVECTOR4(0,0,0,1) - D3DXVECTOR4(m_DirectionalLight.Direction, 1);
		D3DXVECTOR4 normToLight;

		ShaderVariant v;
		v.type = SHADER_VAR_FLOAT4;
		memcpy( v._float4, D3DXVec4Normalize(&normToLight, &toLight), sizeof(v._float4) );
		SetShaderConstant( sp.pVSConstTable, "directionToLight", v );
		
		// TODO: set the lights color and intensities in the shader

		// RENDER STATES

		// fog
		m_device->SetRenderState(D3DRS_FOGCOLOR, D3DCOLOR_XRGB(128,128,128)); // gray
		m_device->SetRenderState(D3DRS_FOGENABLE, TRUE);
	/*	break;

		}*/
	/*case PASS_RTPLAIN:
		SetInvViewport(PASS_RTPLAIN, (float)m_iViewportWidth, (float)m_iViewportHeight);
		m_device->SetRenderState(D3DRS_FOGENABLE, FALSE);
		break;*/
	//case PASS_RTBLURH:
	//	SetInvViewport(PASS_RTBLURH, (float)m_iViewportWidth, (float)m_iViewportHeight);
	//	m_device->SetRenderState(D3DRS_FOGENABLE, FALSE);
	//	break;
	//case PASS_RTBLURV:
	//	SetInvViewport(PASS_RTBLURV, (float)m_iViewportWidth, (float)m_iViewportHeight);
	//	m_device->SetRenderState(D3DRS_FOGENABLE, FALSE);
	//	break;
	//case PASS_PRIMITIVE:
	//case PASS_PRIM_COLORED:
	//	break;
	/*default:
		assert(false);
		break;
	}*/

}

/************************************************
*   Name:   CShaderManager::SetShaderConstant
*   Desc:   Set a provided shader constant to
*           the associated shader program.
************************************************/
bool CShaderManager::SetShaderConstant( ID3DXConstantTable *pConsts, LPCSTR constName, ShaderVariant &variant )
{
	D3DXHANDLE constHnd = pConsts->GetConstantByName( 0, constName );
	if(constHnd)
	{
		switch(variant.type)
		{
		case SHADER_VAR_MATRIX:
			HR(pConsts->SetFloatArray( m_device, constHnd, variant._matrix, 16 ));
			break;
		case SHADER_VAR_FLOAT4:
			{
			HR(pConsts->SetFloatArray( m_device, constHnd, variant._float4, 4 ));
			break;
			}
		case SHADER_VAR_FLOAT:
			HR(pConsts->SetFloat( m_device, constHnd, variant._float ));
			break;
		default:
			assert(false);
			break;
		}
	} else return false;

	return true;
}

/************************************************
*   Name:   CShaderManager::SetWorldTransform
*   Desc:   Set the world transform for the requested
*           shader.
************************************************/
void CShaderManager::SetWorldTransform(EPassId pass, D3DXMATRIX worldTransform)
{
	ShaderVariant &v = m_vecPasses[pass].params.worldTransform;
	v.type = SHADER_VAR_MATRIX;
	memcpy( v._matrix, worldTransform, sizeof(v._matrix) );
	SetShaderConstant( m_vecPasses[pass].pVSConstTable, "matWorld", v );	
}

/************************************************
*   Name:   CShaderManager::SetViewProjection
*   Desc:   Set the ViewProjection transform for
*           the requested shader.
************************************************/
void CShaderManager::SetViewProjection(EPassId pass, D3DXMATRIX viewTransform, D3DXMATRIX projectionTransform )
{ 
	ShaderPass &sp = m_vecPasses[pass]; 
	
	sp.params.viewProjection = viewTransform * projectionTransform;
	sp.params.projection = projectionTransform;
	sp.params.view = viewTransform;

	ShaderVariant v;
	v.type = SHADER_VAR_MATRIX;
	memcpy( v._matrix, sp.params.viewProjection, sizeof(v._matrix) );
	SetShaderConstant( m_vecPasses[pass].pVSConstTable, "matViewProjection", v );

	memcpy( v._matrix, viewTransform, sizeof(v._matrix) );
	SetShaderConstant( m_vecPasses[pass].pVSConstTable, "matView", v );
}

/************************************************
*   Name:   CShaderManager::SetDrawColor
*   Desc:   Set the draw color for the requested
*           shader.
************************************************/
void CShaderManager::SetDrawColor(EPassId pass, D3DXCOLOR color )
{
	ShaderVariant v;
	v.type = SHADER_VAR_FLOAT4;
	memcpy( v._float4, &color, sizeof(v._float4) );
	SetShaderConstant( m_vecPasses[pass].pVSConstTable, "drawColor", v );
}

/************************************************
*   Name:   CShaderManager::SetMaterial
*   Desc:   Set the material attributes for the
*           requested shader.
************************************************/
void CShaderManager::SetMaterial(EPassId pass, const D3DMATERIAL9 &mat ) 
{
	ShaderVariant* v = &m_vecPasses[pass].params.matDiffuse;
	v->type = SHADER_VAR_FLOAT4;
	memcpy( v->_float4, &mat.Diffuse, sizeof(v->_float4) );
	SetShaderConstant( m_vecPasses[pass].pVSConstTable, "diffuseMaterial", *v );
	
	v = &m_vecPasses[pass].params.matAmbient;
	v->type = SHADER_VAR_FLOAT4;
	memcpy( v->_float4, &mat.Ambient, sizeof(v->_float4) );
	SetShaderConstant( m_vecPasses[pass].pVSConstTable, "ambientMaterial", *v );
}

/************************************************
*   Name:   CShaderManager::SetInvViewport
*   Desc:   Set the inverse viewport size for
*           the requested shader.
************************************************/
void CShaderManager::SetInvViewport(EPassId pass, float viewWidth, float viewHeight, bool pixelShader/*=false*/)
{
	float invWidth = 1.0f/viewWidth;
	float invHeight = 1.0f/viewHeight;
	ShaderVariant v;

	ID3DXConstantTable* table = pixelShader ? m_vecPasses[pass].pPSConstTable : m_vecPasses[pass].pVSConstTable;
	v.type = SHADER_VAR_FLOAT;
	v._float = invWidth;
	SetShaderConstant( table, "fInverseViewportWidth", v );

	v._float = invHeight;
	SetShaderConstant( table, "fInverseViewportHeight", v );
}

/************************************************
*   Name:   CShaderManager::PushCurrentShader
*   Desc:   Save the current shader pair on the
*           stack and invalid the current pair.
************************************************/
void CShaderManager::PushCurrentShader()
{
	ShaderStackItem s;
	::ZeroMemory(&s, sizeof(s));
	s.VSPass = m_currentVS;
	s.PSPass = m_currentPS;
	m_currentVS = PASS_INVALID;
	m_currentPS = PASS_INVALID;
	
	m_shaderStack.push(s);
	
}

/************************************************
*   Name:   CShaderManager::PopCurrentShader
*   Desc:   Restore and reload the last 
*           shader pair.
************************************************/
void CShaderManager::PopCurrentShader()
{
//	SetPixelShader(m_shaderStack.top().PSPass);
//	SetVertexShader(m_shaderStack.top().VSPass);
}