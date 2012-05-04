/********************************************************************
	created:	2012/04/16
	filename: 	RenderEngine.cpp
	author:		Matthew Alford
	
	purpose:	Controls render order and manages display lists.
*********************************************************************/
#include "StdAfx.h"
#include "RenderEngine.h"
#include "Hud.h"
#include "vertextypes.h"
#include "Settings.h"

#define WORLD_UP Vector_3(0,1,0)

/*------------------------------------------------------------------------
	Setup Functions
------------------------------------------------------------------------*/
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
	m_texMgr = CTextureManager::GetInstance();
}

CRenderEngine::~CRenderEngine(void)
{
}

/************************************************
*   Name:   CRenderEngine::Setup
*   Desc:   Initializes the render engine and
*           submodules.
************************************************/
bool CRenderEngine::Setup(unsigned int viewportWidth, unsigned int viewportHeight)
{
	m_meshMgr.SetDevice(m_device);

	// Create the render targets
	if(!CreateRenderTarget())
		return false;

	// setup the shader manager constants
	m_shaderMgr.SetViewDimensions(viewportWidth, viewportHeight);
	m_shaderMgr.SetDevice(m_device);

	// setup the texture manager
	m_texMgr->Init();

	/* setup the scene manager */
	m_sceneMgr.Setup(m_meshMgr);

	// Setup the Camera (view matrix)
	m_sceneMgr.GetDefaultCamera().SetViewPort(viewportWidth, viewportHeight);
	m_device->GetRenderTarget(0, &m_surBackBuffer);
	
	return true;
}


/************************************************
*   Name:   CRenderEngine::SetDevice
*   Desc:   Sets the DirectX device handle, and
*           initializes the render engine.
************************************************/
void CRenderEngine::SetDevice(LPDIRECT3DDEVICE9 device, unsigned int viewportWidth, unsigned int viewportHeight) 
{
	m_device = device;
	dxPowerUp( m_device );
	VERIFY(Setup(viewportWidth, viewportHeight));
}

/************************************************
*   Name:   CRenderEngine::SetWireframeMode
*   Desc:   Toggles mesh wireframe rendering.
************************************************/
void CRenderEngine::SetWireframeMode(bool enable)
{
	dxFillMode( enable ? D3DFILL_WIREFRAME : D3DFILL_SOLID );
}

/************************************************
*   Name:   CRenderEngine::CreateRenderTarget
*   Desc:   Initializes the render targets used
*           by the engine.
************************************************/
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
	m_meshMgr.GetGlobalMesh(eScreenQuad, &m_renderQuadGeo );
	assert(m_renderQuadGeo);

	return true;
}

/*------------------------------------------------------------------------
	Render Functions
------------------------------------------------------------------------*/

/************************************************
*   Name:   CRenderEngine::RenderScene
*   Desc:   Main render manager and order.
************************************************/
void CRenderEngine::RenderScene()
{
	/* PASS 0: Render Entire Scene to Texture */
	if(m_nBlurPasses > 0)
		m_device->SetRenderTarget(0, m_surRenderTarget1);
	else
		m_device->SetRenderTarget(0, m_surBackBuffer);
		
	dxClear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 0xff000000/*black*/, 1.0f, 0);

	/* =============================== **
	**      3D Environment View        **
	** =============================== */

	/* DEBUG - Render quadtree cell lines */
	if(Settings_GetBool(DEBUG_DRAW_SCENE_TREE))
	{
		m_sceneMgr.Render( *this );
	}
	
	m_device->BeginScene();
	typedef std::list<IRenderable*> RenderList;

	RenderList opaqueList;
	{// OPAQUE OBJECTS
	dxEnableAlphaBlend(false);
	dxEnableAlphaTest(false);
	dxEnableZWrite();
	dxEnableZTest();
	dxEnableLineAA();
	dxCullMode(D3DCULL_CCW);
	
	m_sceneMgr.GetOpaqueDrawListF2B(opaqueList);
	for(RenderList::iterator it = opaqueList.begin(), _it = opaqueList.end(); it != _it; it++)
	{
		(*it)->Render(*this);
	}

	}

	SceneMgrSortList transList;
	{// TRANSPARENT OBJECTS
	
	// keyed transparency
	dxEnableAlphaTest();
	m_device->SetRenderState(D3DRS_ALPHAREF, 0xff);
	m_device->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	
	// blending
	dxEnableAlphaBlend();
	m_device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	// render state
	m_device->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	m_device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);

	// TODO:: Split up keyed transparencies and alpha blend transparencies for quicker processing

	m_sceneMgr.GetTransparentDrawListB2F(transList);
	for(SceneMgrSortList::iterator it = transList.begin(), _it = transList.end(); it != _it; it++)
	{
		(*it).p->Render(*this);
	}

	m_device->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
	m_device->SetRenderState(D3DRS_ALPHAREF, 0xff);
	m_device->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_LESS);
	m_device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

	for(SceneMgrSortList::iterator it = transList.begin(), _it = transList.end(); it != _it; it++)
	{
		(*it).p->Render(*this);
	}

	}

	/* (DEBUG) - Render the sphere camera clipping geometry for each scene object */
	if(Settings_GetBool(DEBUG_SHOW_CLIP_BOUNDS))
	{
		ColorRGBA32 clr = {0.5f, 0.1f, 0.1f, 0.45f};
		SceneMgrSortList sphereList;
		m_sceneMgr.GetAllObjectsDrawListB2F(sphereList);
		sphereList.reverse();

		for(SceneMgrSortList::iterator it = sphereList.begin(), _it = sphereList.end(); it != _it; it++)
		{
			Sphere_PosRad sphere = it->p->GetBoundingSphere();	
			DrawDebugSphere(sphere, clr);
		}

		Vector_3 orig;
		orig.x = 0;
		orig.y = 0;
		orig.z = 0;
		DrawDebugAxes(orig);
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
	if(Settings_GetBool(HUD_SHOW_HUD))
	{
		m_device->BeginScene();
		m_pHud->Render();
		m_device->EndScene();
	}
}

/*------------------------------------------------------------------------
	Update Functions
------------------------------------------------------------------------*/

/************************************************
*   Name:   CRenderEngine::Update
*   Desc:   Updates modules owned by Render Engine
************************************************/
void CRenderEngine::Update( float elapsedMillis )
{
	m_shaderMgr.Update(elapsedMillis);
	m_meshMgr.Update(elapsedMillis);
}

/*------------------------------------------------------------------------
	Debug Draw Functions
------------------------------------------------------------------------*/

/************************************************
*   Name:   CRenderEngine::DrawDebugSphere
*   Desc:   Draws a colored 3D sphere.
************************************************/
void CRenderEngine::DrawDebugSphere(Sphere_PosRad& sphere, ColorRGBA32 color)
{
	Matrix4x4 scaling, translation, world;

	float r = sphere.radius;
	Matrix4x4_Scale(&scaling, r, r, r);
	Matrix4x4_Translate(&translation, sphere.pos.x, sphere.pos.y, sphere.pos.z);

	world = scaling * translation;

	BaseModel *model;
	m_meshMgr.GetGlobalMesh(eUnitSphere, &model);

	if( color.a < 1.0f )
	{
		model->SetRenderFunc(model->RenderFuncs.xpLightAndColored);
	}
	else
	{
		model->SetRenderFunc(model->RenderFuncs.lightAndColored);
	}

	model->SetDrawColor(color);
	model->Render(*this, world, m_shaderMgr);
}

/************************************************
*   Name:   CRenderEngine::DrawDebugAxes
*   Desc:   Draws an XYZ axis at the given location
*           Axis is rotated to face world orientation.
************************************************/
void CRenderEngine::DrawDebugAxes(Vector_3 location)
{
	/* draw x axis red */
	ColorRGBA32 xclr;
	::ZeroMemory(&xclr, sizeof(xclr));
	xclr.a = 1.0f;
	xclr.r = 1.0f;

	/* draw y axis green */
	ColorRGBA32 yclr;
	::ZeroMemory(&yclr, sizeof(yclr));
	yclr.a = 1.0f;
	yclr.g = 1.0f;

	/* draw z axis blue */
	ColorRGBA32 zclr;
	::ZeroMemory(&zclr, sizeof(zclr));
	zclr.a = 1.0f;
	zclr.b = 1.0f;

	/* end points */
	Vector_3 xend, yend, zend;
	xend = yend = zend = location;
	xend.x += 1.0f;
	yend.y += 1.0f;
	zend.z += 1.0f;

	/* draw lines */
	DrawDebugLine3D(location, xend, xclr, true);
	DrawDebugLine3D(location, yend, yclr, true);
	DrawDebugLine3D(location, zend, zclr, true);
}

/************************************************
*   Name:   CRenderEngine::DrawDebugLine3D
*   Desc:   Draws a 3D colored line from given 
*           start to end.  Optionally draws the
*           spherical end-points.
************************************************/
void CRenderEngine::DrawDebugLine3D(Vector_3 start, Vector_3 end, ColorRGBA32 color, bool showPoints/*= false*/)
{
	static const float thickness = 0.05f;

	Matrix4x4 world;
	Matrix4x4_LoadIdentity(&world);
	BaseModel *rod;
	m_meshMgr.GetGlobalMesh(eUnitCylinder, &rod);

	if(showPoints)
	{
		Sphere_PosRad s;
		s.pos = start;
		s.radius = 0.05f;
		DrawDebugSphere(s, color);
		s.pos = end;
		DrawDebugSphere(s, color);
	}
	
	/* find the middle point */
	Vector_3 mid;
	mid.x = (start.x + end.x) * 0.5f;
	mid.y = (start.y + end.y) * 0.5f;
	mid.z = (start.z + end.z) * 0.5f;

	/* stretch the rod to cover the gap between points */
	Vector_3 startend = end - start;
	float len = Vec3_Length(&startend);
	Matrix4x4_Scale(&world, thickness, thickness, len);

	/* rotate the rod to be the direction formed by line between two provided points */
	Matrix4x4 xfm;
	Vector_3 up = Vector_3(0,0,1);
	Vector_3 axis;
	Vector_3 dir = end - start;
	Vec3_Normalize(&dir, &dir);
	Vec3_Cross(&axis, &up, &dir);
	float rotation = acos(Vec3_Dot(&up, &dir));
	
	D3DXMatrixRotationAxis( &xfm, &axis, rotation );

	world *= xfm;

	/* move the rod into place, midway between the points */
	Matrix4x4_Translate(&xfm, mid.x, mid.y, mid.z);
	world *= xfm;

	/* draw rod */
	rod->SetDrawColor(color);
	rod->SetRenderFunc(rod->RenderFuncs.lightAndColored);
	rod->Render(*this, world, m_shaderMgr);
}