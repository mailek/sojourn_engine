#include "StdAfx.h"
#include "RenderEngine.h"
#include "Hud.h"
#include "vertextypes.h"

//////////////////////////////////////////////////////////////////////////
// Setup Functions
//////////////////////////////////////////////////////////////////////////

CRenderEngine::CRenderEngine(void) : m_device(NULL),
									m_renderQuadGeo(NULL),
									m_texRenderTarget1(NULL),
									m_texRenderTarget2(NULL),
									m_texRenderTarget3(NULL),
									m_surRenderTarget1(NULL),
									m_surRenderTarget2(NULL),
									m_surRenderTarget3(NULL),
									m_surBackBuffer(NULL),
									m_nBlurPasses(0),
									m_pHud(NULL),
									m_bShowHUD(false)
{
}

CRenderEngine::~CRenderEngine(void)
{
}

bool CRenderEngine::Setup(unsigned int viewportWidth, unsigned int viewportHeight)
{
	m_meshMgr.SetDevice(m_device);

	// Create the render targets
	if(!CreateRenderTarget())
		return false;

	// setup the shader manager constants
	m_shaderMgr.SetViewDimensions(viewportWidth, viewportHeight);
	m_shaderMgr.SetDevice(m_device);

	// Setup the Camera (view matrix)
	m_sceneMgr.GetDefaultCamera().SetViewPort(viewportWidth, viewportHeight);
	m_device->GetRenderTarget(0, &m_surBackBuffer);
	
	return true;
}

void CRenderEngine::SetDevice(LPDIRECT3DDEVICE9 device, unsigned int viewportWidth, unsigned int viewportHeight) 
{
	m_device = device;
	bool success = Setup(viewportWidth, viewportHeight);
	assert(success);
}

void CRenderEngine::SetWireframeMode(bool enable)
{
	if(m_device == NULL)
		return;

	if(enable)
		m_device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	else
		m_device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
}

bool CRenderEngine::CreateRenderTarget(void)
{
	// Create the RT's texture and surface
	HR(m_device->CreateTexture(SCREEN_RES_H, SCREEN_RES_V, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_texRenderTarget1, NULL));
	HR(m_texRenderTarget1->GetSurfaceLevel(0, &m_surRenderTarget1));
	
	HR(m_device->CreateTexture(SCREEN_RES_H, SCREEN_RES_V, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_texRenderTarget2, NULL));
	HR(m_texRenderTarget2->GetSurfaceLevel(0, &m_surRenderTarget2));
	
	// create the downsample rendertarget
	HR(m_device->CreateTexture(SCREEN_RES_H/4, SCREEN_RES_V/4, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_texRenderTarget3, NULL));
	HR(m_texRenderTarget3->GetSurfaceLevel(0, &m_surRenderTarget3));

	// Create the render target quad geo
	m_meshMgr.GetGlobalMesh(CMeshManager::eScreenQuad, &m_renderQuadGeo );
	assert(m_renderQuadGeo);

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Render Functions
//////////////////////////////////////////////////////////////////////////

void CRenderEngine::RenderScene()
{
	// PASS 0: Render Entire Scene to Texture
	if(m_nBlurPasses > 0)
		m_device->SetRenderTarget(0, m_surRenderTarget1);
	else
		m_device->SetRenderTarget(0, m_surBackBuffer);
		
	m_device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00000000/*black*/, 1.0f, 0);
	
	// QUAD TREE RENDER (DEBUG)
	m_sceneMgr.Render( *this ); // render the debug quad tree

	m_shaderMgr.SetVertexShader(PASS_DEFAULT);
	m_shaderMgr.SetPixelShader(PASS_DEFAULT);
	
	CCamera& camera = m_sceneMgr.GetDefaultCamera();
	D3DXMATRIX viewMatrix = camera.GetViewMatrix();
	m_shaderMgr.SetViewProjection(PASS_DEFAULT, viewMatrix, camera.GetProjectionMatrix());
		
	m_device->BeginScene();
	typedef std::list<IRenderable*> RenderList;

	{// OPAQUE OBJECTS
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	m_device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	m_device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	m_device->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	
	RenderList opaqueList;
	m_sceneMgr.GetOpaqueDrawListF2B(opaqueList);
	for(RenderList::iterator it = opaqueList.begin(), _it = opaqueList.end(); it != _it; it++)
		(*it)->Render(*this);
	}
	
	{// TRANSPARENT OBJECTS
	//m_device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	m_device->SetRenderState( D3DRS_ALPHATESTENABLE, TRUE); 
	m_device->SetRenderState( D3DRS_ALPHAREF, 0x16);
	m_device->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
	m_device->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE);
	m_device->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE);
	m_device->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_device->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

	SceneMgrSortList transList;
	m_sceneMgr.GetTransparentDrawListB2F(transList);
	for(SceneMgrSortList::iterator it = transList.begin(), _it = transList.end(); it != _it; it++)
		(*it).p->Render(*this);
	}

	m_device->EndScene();

	// BLUR PASSES
	if(m_nBlurPasses > 0)
	{
		m_device->BeginScene();

		const LPDIRECT3DTEXTURE9 tex[] = { m_texRenderTarget1, m_texRenderTarget2 };
		const LPDIRECT3DSURFACE9 rt[] = { m_surRenderTarget1, m_surRenderTarget2 };
		LPDIRECT3DVERTEXBUFFER9 vertices(NULL);
		m_renderQuadGeo->GetVertexBuffer( &vertices );
		m_device->SetStreamSource(0, vertices, 0, sizeof(RenderQuadVertex));
		m_shaderMgr.SetEffect(EFFECT_GAUSSBLUR);

		//downsample
		//m_shaderMgr.SetTechnique("Downsample");

		//apply the blur
		m_shaderMgr.SetTechnique("GaussBlur");
		for(int i=0, lastBlur=m_nBlurPasses-1; i<m_nBlurPasses; i++)
		{
			int passes = m_shaderMgr.BeginEffect();
			for( int j = 0, lastPass=passes-1; j <passes; j++ )
			{
				LPDIRECT3DTEXTURE9 inputTex = tex[(i+j)%2];
				LPDIRECT3DSURFACE9 outputTarget = rt[(i+j+1)%2];
				if( i == lastBlur && j == lastPass )
					outputTarget = m_surBackBuffer;
				
				m_shaderMgr.SetTexture("tex0", inputTex);
				m_device->SetTexture(0, inputTex);
				m_device->SetRenderTarget(0, outputTarget);
				
				m_shaderMgr.Pass(j);
				m_device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);
				m_shaderMgr.FinishPass();
			}
			m_shaderMgr.FinishEffect();

		}

		m_device->EndScene();
		COM_SAFERELEASE(vertices);
	}

	// HUD PASS
	m_shaderMgr.SetVertexShader(PASS_DEFAULT);
	m_shaderMgr.SetPixelShader(PASS_DEFAULT);
	viewMatrix = camera.GetViewMatrix();
	m_shaderMgr.SetViewProjection(PASS_DEFAULT, viewMatrix, camera.GetProjectionMatrix());

	m_device->BeginScene();

	m_device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	m_device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	m_device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

	if(m_bShowHUD)
		m_pHud->Render();

	m_device->EndScene();
}

//////////////////////////////////////////////////////////////////////////
// Update Functions
//////////////////////////////////////////////////////////////////////////

void CRenderEngine::Update( LPDIRECT3DDEVICE9 device, float elapsedMillis)
{
	m_sceneMgr.GetDefaultCamera().Update(elapsedMillis);
	m_shaderMgr.Update(elapsedMillis);
	m_meshMgr.Update( device, elapsedMillis);
}
