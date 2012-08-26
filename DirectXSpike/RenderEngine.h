#pragma once
/********************************************************************
	created:	2012/04/16
	filename: 	RenderEngine.h
	author:		Matthew Alford
	
	purpose:	
*********************************************************************/
//////////////////////////////////////
// Includes
//////////////////////////////////////
#include "TerrainPatch.h"
#include "Skybox.h"
#include "MeshManager.h"
#include "ShaderManagerEx.h"
#include "SceneManager.h"
#include "LevelManager.h"
#include "TextureManager.h"

//////////////////////////////////////
// Forward Declarations
//////////////////////////////////////
class CHUD;

//////////////////////////////////////
// Type Definitions
//////////////////////////////////////

//////////////////////////////////////
// Class Definition
//////////////////////////////////////
class CRenderEngine
{
public:
	CRenderEngine(void);
	~CRenderEngine(void);

private:
	CMeshManager					m_meshMgr;
	CShaderManagerEx				m_shaderMgr;
	CLevelManager					m_levelMgr;
	CSceneManager					m_sceneMgr;
	CTextureManager*				m_texMgr;
	
	int								m_nBlurPasses;

	LPDIRECT3DDEVICE9				m_device;
	BaseModel*						m_renderQuadGeo;
	LPDIRECT3DTEXTURE9				m_texRenderTarget1;		/* texture for render target 1 */
	LPDIRECT3DTEXTURE9				m_texRenderTarget2;		/* texture for render target 2 */
	LPDIRECT3DTEXTURE9				m_texRenderTarget3;     /* texture for render target 3 */
	LPDIRECT3DSURFACE9				m_surRenderTarget1;		/* texture surface for render target 1 */
	LPDIRECT3DSURFACE9				m_surRenderTarget2;		/* texture surface for render target 2 */
	LPDIRECT3DSURFACE9				m_surRenderTarget3;		/* texture surface for render target 3 */
	LPDIRECT3DSURFACE9				m_surBackBuffer;		/* back buffer for restoring after render target pass */
//	LPDIRECT3DTEXTURE9				m_texTest;				/* render target testing */
	CHUD*							m_pHud;					/* pointer to HUD manager */

	void Setup(unsigned int viewportWidth, unsigned int viewportHeight);

public:
	void DrawDebugSphere(Sphere_PosRad& sphere, Color_4 color);
	void DrawDebugLine3D(Vector_3 start, Vector_3 end, Color_4 color, bool showPoints = false);
	void DrawDebugAxes(Vector_3 location);

	void SetDevice(LPDIRECT3DDEVICE9 device, unsigned int viewportWidth, unsigned int viewportHeight);
	void RenderScene();
	void SetWireframeMode(bool enable);
	void SetPlayerObject(CPlayer* pPlayer);
	void Update(float elapsedMillis);
	
	inline CMeshManager &GetMeshManager() {return m_meshMgr;}
	inline CTextureManager &GetTextureManager() {return *m_texMgr;}
	inline void SetBlur(int blurs) {m_nBlurPasses = blurs;}
	inline void SetHUD(CHUD *hud) {m_pHud = hud;}
	inline D3DXVECTOR3 GetLightDirection(void) {return m_shaderMgr.GetLightDirection();}

	inline void ToggleClipMethod() {m_sceneMgr.SetNextClipStrategy();}
	inline LPDIRECT3DDEVICE9 GetDevice() {return m_device;}
	inline CSceneManager* GetSceneManager() {return &m_sceneMgr;}
	inline CShaderManagerEx &GetShaderManager() {return m_shaderMgr;}
	
	void CreateRenderTarget();
};