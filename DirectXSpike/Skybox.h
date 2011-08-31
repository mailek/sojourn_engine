#pragma once
//////////////////////////////////////////////////////////////////////////
// Skybox.h - 2011 Matthew Alford
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////
// Includes
//////////////////////////////////////
#include "IRenderable.h"
#include "basemodel.h"

//////////////////////////////////////
// Forward Declarations
//////////////////////////////////////
class CShaderManager;
class CCamera;

//////////////////////////////////////
// Class Definition
//////////////////////////////////////
class CSkybox : public IRenderable
{
public:
	CSkybox(void);
	~CSkybox(void);
	bool LoadSkyDome(LPDIRECT3DDEVICE9 device, float radius, UINT slices, UINT stacks);
	inline void SetCameraObjectToFollow(CCamera* camera) {m_pCamera = camera;}
	
	virtual void Render( CRenderEngine &rndr );
	virtual D3DXMATRIX GetWorldTransform();
	virtual bool IsTransparent() {return false;}
	virtual void SetLastRenderFrame(UINT frameNum) {};
	virtual UINT GetLastRenderFrame() {return 0;}
	virtual Sphere_PosRad GetBoundingSphere() { Sphere_PosRad s; ::ZeroMemory(&s, sizeof(s)); return s; }

private:
	LPDIRECT3DTEXTURE9		m_texSkyTexture;
	LPD3DXMESH				m_mesh;
	//BaseModel				m_baseMesh;
	D3DMATERIAL9			m_skyMaterial;
	CCamera*				m_pCamera;			// pointer to camera object to follow
	float					m_sphereRadius;

};