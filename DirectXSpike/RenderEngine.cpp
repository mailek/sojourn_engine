#include "StdAfx.h"
#include "RenderEngine.h"
#include "Hud.h"
#include "vertextypes.h"
#include "Settings.h"

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
									m_pHud(NULL)
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

	/* =============================== **
	**      3D Environment View        **
	** =============================== */

	// ** DEBUG ** 
	// Render quadtree cell lines
	bool settingBool;
	Settings_GetBool(DEBUG_DRAW_SCENE_TREE, settingBool);
	if( settingBool == true)
	{
		m_sceneMgr.Render( *this );
	}

	m_shaderMgr.SetVertexShader(PASS_DEFAULT);
	m_shaderMgr.SetPixelShader(PASS_DEFAULT);
	
	CCamera& camera = m_sceneMgr.GetDefaultCamera();
	D3DXMATRIX viewMatrix = camera.GetViewMatrix();
	m_shaderMgr.SetViewProjection(PASS_DEFAULT, viewMatrix, camera.GetProjectionMatrix());
		
	m_device->BeginScene();
	typedef std::list<IRenderable*> RenderList;

	RenderList opaqueList;
	{// OPAQUE OBJECTS
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	m_device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	m_device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	m_device->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	
	m_sceneMgr.GetOpaqueDrawListF2B(opaqueList);
	for(RenderList::iterator it = opaqueList.begin(), _it = opaqueList.end(); it != _it; it++)
		(*it)->Render(*this);
	}

	SceneMgrSortList transList;
	{// TRANSPARENT OBJECTS
	
	// keyed transparency
	m_device->SetRenderState( D3DRS_ALPHATESTENABLE, TRUE); 
	m_device->SetRenderState( D3DRS_ALPHAREF, 0x16);
	m_device->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
	m_device->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	
	// blending
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	m_device->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE);
	//m_device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

	// TODO:: Split up keyed transparencies and alpha blend transparencies for quicker processing

	m_sceneMgr.GetTransparentDrawListB2F(transList);
	for(SceneMgrSortList::iterator it = transList.begin(), _it = transList.end(); it != _it; it++)
		(*it).p->Render(*this);
	}

	// ** (DEBUG) **
	// Render the sphere camera clipping geometry for each scene object
	bool showClipBounds = false;
	Settings_GetBool(DEBUG_SHOW_CLIP_BOUNDS, showClipBounds);
	if(showClipBounds)
	{
		m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		m_device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
		m_device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

		Matrix4x4 world;
		CDoubleLinkedList<IRenderable> sphereList;
		for(RenderList::iterator it = opaqueList.begin(), _it = opaqueList.end(); it != _it; it++)
			sphereList.AddItemToEnd(*it);

		for(SceneMgrSortList::iterator it = transList.begin(), _it = transList.end(); it != _it; it++)
			sphereList.AddItemToEnd(it->p);

		ColorRGBA32 clr;
		clr.r = 1.0f;
		clr.g = 1.0f;
		clr.b = 1.0f;
		clr.a = 0.4f;
		for(CDoubleLinkedList<IRenderable>::DoubleLinkedListItem* s = sphereList.first; s != NULL; s = s->next)
		{
			Sphere_PosRad sphere = s->item->GetBoundingSphere();
			DrawDebugSphere(sphere, clr);
		}

		m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		for(CDoubleLinkedList<IRenderable>::DoubleLinkedListItem* s = sphereList.first; s != NULL; s = s->next)
		{
			Sphere_PosRad sphere = s->item->GetBoundingSphere();
			DrawDebugSphere(sphere, clr);
		}

	}

	m_device->EndScene();

	/* =============================== **
	**        Post Processing          **
	** =============================== */

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

	/* =============================== **
	**              HUD                **
	** =============================== */

	// HUD PASS
	m_shaderMgr.SetVertexShader(PASS_DEFAULT);
	m_shaderMgr.SetPixelShader(PASS_DEFAULT);
	viewMatrix = camera.GetViewMatrix();
	m_shaderMgr.SetViewProjection(PASS_DEFAULT, viewMatrix, camera.GetProjectionMatrix());

	m_device->BeginScene();

	m_device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	m_device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	m_device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

	bool settingsBool;
	Settings_GetBool(HUD_SHOW_HUD, settingsBool);
	if(settingsBool == true)
	{
		m_pHud->Render();
	}

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


//////////////////////////////////////////////////////////////////////////
// Debug Draw Functions
//////////////////////////////////////////////////////////////////////////
void CRenderEngine::DrawDebugSphere(Sphere_PosRad& sphere, ColorRGBA32 color)
{
	Matrix4x4 scaling, translation, world;

	float r = sphere.radius;
	Matrix4x4_Scale(&scaling, r, r, r);
	Matrix4x4_Translate(&translation, sphere.pos.x, sphere.pos.y, sphere.pos.z);

	world = scaling * translation;

	BaseModel *model;
	m_meshMgr.GetGlobalMesh(CMeshManager::eUnitSphere, &model);

	model->SetDrawColor(color);
	model->Render(m_device, world, m_shaderMgr);
}